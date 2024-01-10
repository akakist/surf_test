#include "surf.h"
#include <fstream>
#include <iostream>
#include <deque>
/// используется для вычисления максимальной длины ребра
#define MAX_N 15

/// минимальмная сумма прилежащих точке готовых углов треугольников,
/// чтобы понять, что угол в точке является вогнутым
#define MIN_SUM_ANGLE_TRIANGLES 210

/// максимальный угол между нормалями нового треугольника и старого при добавлении новой точки
#define MAX_ANGLE_BETWEEN_NORMALES 50

/// векторное произведение
Point cross(const Point& a,const Point & v)
{
    return Point(a.y*v.z - a.z*v.y, a.z*v.x - a.x*v.z, a.x*v.y - a.y*v.x);
}

/// проверка на максимальную длину линии
bool surface::line_len_ok(double len)
{
//    return true;
    return len < avg_dist*MAX_N;
}

/// вставка треугольника и коррекция всех необходимых контейнеров
/// в случае ошибки возвращаем нулл
std::shared_ptr<triangle> surface::make_tiangle(int p0, int p2, int p3)
{
    auto &pi0=pointInfos[p0];
    auto &pi2=pointInfos[p2];
    auto &pi3=pointInfos[p3];

    /// проверяем на максимальный размер стороны треугольника
    auto d02=fdist(pts[p0],pts[p2]);
    auto d03=fdist(pts[p0],pts[p3]);
    auto d23=fdist(pts[p2],pts[p3]);
    auto max=std::max(d02,std::max(d03,d23));
    if(!line_len_ok(max))
    {
        return nullptr;
    }

    /// берем ребра
    auto r02=get_edge_or_create({p0,p2},"link_neighbours2");
    auto r03=get_edge_or_create({p0,p3},"link_neighbours2");
    auto r23=get_edge_or_create({p2,p3},"link_neighbours2");

    if(r02->opposize_pts.size()>1 || r03->opposize_pts.size()>1 || r23->opposize_pts.size()>1)
    {
        return nullptr;
    }
    /// добавляем к точкам ребра
    pi0.add_to_edges(r02);
    pi0.add_to_edges(r03);
    pi2.add_to_edges(r02);
    pi2.add_to_edges(r23);
    pi3.add_to_edges(r03);
    pi3.add_to_edges(r23);

    /// добавляем точки к соседям точки
    pi0.add_neighbours({p2,p3});
    pi2.add_neighbours({p0,p3});
    pi3.add_neighbours({p0,p2});

    /// добавляем к ребрам третью точку треугольника. Eсли их 2, то ребро полностью заполнено
    r02->add_opposite_pts(p3);
    r03->add_opposite_pts(p2);
    r23->add_opposite_pts(p0);

    /// если ребра полностью заполнены, то они далее не нужны, чистим их где можно
    if(r02->opposize_pts.size()==2)
    {
        pi0.neighbours.erase(p2);
        pi2.neighbours.erase(p0);
    }
    if(r03->opposize_pts.size()==2)
    {
        pi0.neighbours.erase(p3);
        pi3.neighbours.erase(p0);
    }
    if(r23->opposize_pts.size()==2)
    {
        pi2.neighbours.erase(p3);
        pi3.neighbours.erase(p2);
    }

    /// создаем треугольник
    /// не используем std::make_shared, поскольку он почему-то не может передать в конструктор  std::initializer_list
    std::shared_ptr<triangle> t(new triangle({p0,p2,p3}));
    /// к треугольнику добавляем ребра
    t->edges.insert({r02,r03,r23});
    t->center=(pts[p0]+pts[p2]+pts[p3])/3.;
//    surface_pts.push_back((pts[p0]+pts[p2]+pts[p3])/3.);
//    printf("surface_pts size %d\n",surface_pts.size());
//    surface_pts.push_back(pts[p0]);
//    surface_pts.push_back(pts[p2]);
//    surface_pts.push_back(pts[p3]);
//    printf("t center %lf %lf %lf\n",t->center.x,t->center.y,t->center.z);
//    all_triangles.insert({t->id,t});


    /// дибавляем к точкам треугольник
    pi0.triangles.insert(t);
    pi2.triangles.insert(t);
    pi3.triangles.insert(t);
    /// добавляем к контейнеру треугольников, который будет использован для output
    if(!triangles.count({p0,p2,p3}))
    {
        if(triangles.size()%1000==0 && triangles.size()!=0)
            std::cout<<"triangles "<< triangles.size()<< std::endl;
        triangles.insert({p0,p2,p3});
    }
    else
        throw std::runtime_error("triangle already inserted %s "+dump_set_int({p0,p2,p3}));

    linked_points.insert({p0,p2,p3});

    return t;

}


/// нахождение ближайшей точки из сета, как правило из unlinked_points
int surface::find_nearest(const Point& pt, const std::set<int> &ps)
{

    auto min=std::numeric_limits<double>::max();
    int sel=-1;
    for(auto& p:ps)
    {
        auto d=fdist(pt,pts[p]);
        if(d<min && line_len_ok(d))
        {
            min=d;
            sel=p;
        }
    }
    return sel;
}


/// нахождение ближайшей точки с использованием условия triangle_can_be_added
int surface::find_nearest_which_can_be_added(const Point& pt,  int p0,int p2, int p_opposite)
{
    auto min=std::numeric_limits<double>::max();
    int sel_unlinked=-1;
    std::set<int> erased_pts;
    for(auto& p:unlinked_points)
    {
        auto d=fdist(pt,pts[p]);
        if(erased_pts.count(p))
            continue;
        if(d<min && line_len_ok(d) && triangle_can_be_added(p0,p2,p,p_opposite,d,erased_pts))
        {
            min=d;
            sel_unlinked=p;
        }
    }
    for(auto& p:erased_pts)
    {
        unlinked_points.erase(p);
    }

    return sel_unlinked;

}


/// угол между векторами из точки root в а и б
double surface::angle_between_3_points(int root,int a, int b)
{
    auto& p_root=pts[root];
    auto &p_a=pts[a];
    auto &p_b=pts[b];
    return Angle::angle(p_a-p_root,p_b-p_root);
}

/// векторное произведение векторов из точки root в а и б
Point surface::cross_between_3_points(int root,int a, int b)
{
    auto &p_root=pts[root];
    auto &p_a=pts[a];
    auto &p_b=pts[b];
    return cross(p_a-p_root,p_b-p_root);
}

/// оценка допустимости добавления треугольника по нормалям 2 треугольников p0,p2,p_nearest и p0,p2, p_opposite
bool surface::triangle_can_be_added(int p0, int p2, int p_nearest, int p_opposite, double distance, std::set<int>& erased_pts)
{

    return true;

    /// не используем метод нормалей
    /// вычисляем нормали к плоскостям {p2->p22_nearest,p2->p0} и к {p2->p0, p2->p_opp_r2}
    /// они сонаправлены и угол между ними должен быть < 90град
    auto normal2=cross_between_3_points(p2, p_nearest,p0);
    auto normalB2=cross_between_3_points(p2,p0,p_opposite);
    auto a=Angle::angle(normal2,normalB2);
    if(isnan(a))
        return false;
    if(a<MAX_ANGLE_BETWEEN_NORMALES)
    {
        return true;
    }
    return false;

}

/// если угол между граничными ребрами вогнутый по отношению к поверхности, то соединяем концы ребер новым ребром
int surface::proceed_connection_between_tops(int p0)
{
    auto& pi0=pointInfos[p0];
    /// берем ребра, у которых треугольник есть только с одной стороны
    auto notfilled=pi0.not_filles_rebras();

    if(notfilled.size()==2)
    {
        /// их количество должно быть для точки ровно 2, чтобы точка была на границе заполнения
        auto r2=*notfilled.begin();
        auto r3=*notfilled.rbegin();
        /// берем точки ребер и вычищаем из них точку p0 тем самым получаем граничные точки p2,p3 с которым соседствует точка p0
        auto s2=r2->points;
        auto s3=r3->points;
        s2.erase(p0);
        s3.erase(p0);
        if(s2.size()!=1 || s3.size()!=1)
            throw std::runtime_error("if(s2.size()==0 || s3.size()==0)");
        auto p2=*s2.begin();
        auto p3=*s3.begin();

        /// далее берем все существующие треугольники, в которых точка p0 является вершиной
        /// Далее берем сумму всех углов треугольников от точки p0 и если сумма углов треугольников >MAX_SUM_ANGLE_TRIANGLES,
        /// то считаем, что линия вогнута и мы можем соединить точки p2,p3 и создать новый треугольник p0,p2,p3
        double sum_angles=0;
        for(auto&t: pi0.triangles)
        {
            auto ps=t->points;
            ps.erase(p0);
            if(ps.size()!=2)
                throw std::runtime_error("if(ps.size()!=2) "+dump_set_int(ps));

            int a=*ps.begin();
            int b=*ps.rbegin();
            sum_angles+=angle_between_3_points(p0,a,b);
        }
        if(sum_angles>MIN_SUM_ANGLE_TRIANGLES)
        {
            auto t=make_tiangle(p0,p2,p3);
            /// удаляем p0 из active_points поскольку она перестала быть граничной.
            if(t.get()!=nullptr)
            {
                active_points.erase(p0);

                return 1;
            }
        }
        else
        {
        }

    }
    return 0;

}

/// если точка граничит с двумя незаполненными ребрами (граничит с одним треугольником),
/// то добавляем к ним новую ближайшую точку.
void surface::proceed_add_new_point_between_edges(int p0, std::deque<int>& interested)
{
    auto& pi0=pointInfos[p0];
    /// берем ребра, у которых треугольник есть только с одной стороны
    auto notfilled=pi0.not_filles_rebras();

    std::vector<int> ret;
    if(notfilled.size()!=2)
    {
                throw std::runtime_error("if(notfilled.size()!=2) ");
    }

        /// их количество должно быть для точки ровно 2, чтобы точка была на границе заполнения
        auto r2=*notfilled.begin();
        auto r3=*notfilled.rbegin();
        /// берем точки ребер и вычищаем из них точку p0 тем самым получаем граничные точки p2,p3 с которым соседствует точка p0
        auto s2=r2->points;
        auto s3=r3->points;
        s2.erase(p0);
        s3.erase(p0);
        if(s2.size()!=1 || s3.size()!=1)
            throw std::runtime_error("if(s2.size()==0 || s3.size()==0)");
        auto p2=*s2.begin();
        auto p3=*s3.begin();

        //// add unlinked point to both rebras
        auto center2=(pts[p0]+pts[p2])/2;
        auto center3=(pts[p0]+pts[p3])/2;
        if(r2->opposize_pts.size()!=1)
            throw std::runtime_error("if(r2->opposize_pts.size()!=1)");
        if(r3->opposize_pts.size()!=1)
            throw std::runtime_error("if(r2->opposize_pts.size()!=1)");
        int p_opp_r2=*r2->opposize_pts.begin();
        int p_opp_r3=*r2->opposize_pts.begin();

        auto p22_nearest=find_nearest_which_can_be_added(center2,p0,p2,p_opp_r2);
        auto p33_nearest=find_nearest_which_can_be_added(center3,p0,p3,p_opp_r3);

        /// если p22_nearest и p33_nearest совпадают, то строим все равно 2 треугольника и потом убираем ее из анлинкед поинтс
        ///
        /// берем точку, оппозитную ребру {p0,p2}, она одна, поскольку ребро не заполнено с обеих сторон
        if(p22_nearest!=-1)
        {
            if(make_tiangle(p2,p0,p22_nearest).get()!=nullptr)
            {
                unlinked_points.erase(p22_nearest);
                active_points.insert(p22_nearest);
                interested.push_back(p2);
            }
        }
        if(p33_nearest!=-1) {
            /// тоже самое с другой стороной
            if(make_tiangle(p3,p0,p33_nearest).get()!=nullptr)
            {
                unlinked_points.erase(p33_nearest);
                active_points.insert(p33_nearest);
                interested.push_back(p2);
            }
        }

        /// если ближайшие точки не удовлетворяют условию, то оставляем их в unlinked_points,  надеясь, что их используют другие ребра

        /// в случае совпадения p22_nearest и p33_nearest p0 попадает внутрь закрашенного региона, поэтому удаляем ее из active_points
        if(pi0.not_filles_rebras().size()==0)
        {
            active_points.erase(p0);
        }

        /////////////////////
}

/// главная функция алгоритма заполнения
void surface::flood()
{
    /// изначально складываем все точки в контейнер unlinked_points
    for(int i=0; i<pts.size(); i++)
    {
        unlinked_points.insert(i);
    }
    /// создаем первый треугольник на произвольной точке
    if(unlinked_points.empty())
        throw std::runtime_error("if(unlinked_points.empty()) 1");
    int p0=*unlinked_points.begin();
    unlinked_points.erase(p0);
    if(unlinked_points.empty())
        throw std::runtime_error("if(unlinked_points.empty()) 2");
    int p2=find_nearest(pts[p0],unlinked_points);
    unlinked_points.erase(p2);
    if(unlinked_points.empty())
        throw std::runtime_error("if(unlinked_points.empty()) 3");
    int p3=find_nearest(pts[p0],unlinked_points);
    unlinked_points.erase(p3);

    /// создаем треугольник
    auto t=make_tiangle(p0,p2,p3);
    if(t.get()!=nullptr)
    {
        active_points.insert({p0,p2,p3});
    }
    /// end 1st triangle

    bool cond=true;
    while(cond)
    {
        /// в цикле добавляем новые точки, пока они добавляются
        /// затем соединяем ребрами острые углы и так далее, пока они соединяются
        auto a=active_points;

        /// в ходе заполнения появляются потенциально выпуклые острые углы, которые можно проверить и соединить линией
        /// их складываем в interested
        std::deque<int> interested;
        for(auto& p:a)
        {
            proceed_add_new_point_between_edges(p,interested);
        }
        for(auto& p:interested)
        {
            proceed_connection_between_tops(p);
        }
        int connected=0;
        if(interested.size()==0)
        {
            for(auto& p:a)
            {
                connected+=proceed_connection_between_tops(p);
            }
        }
        if(interested.size()==0 && connected==0)
            cond=false;


    }

}


void surface::run(const std::string &fn_in, const std::string& fn_out)
{

    load_points(fn_in);
    calc_picture_size();
    pointInfos.resize(pts.size());

    avg_dist=picture_size/sqrt(pts.size());

    std::cout << "Calculate triangles" <<std::endl;

    flood();

    std::cout << "triangles " << triangles.size() << std::endl;

    std::cout << "Done triangles" <<std::endl;

    std::cout << "Print out" << std::endl;
    std::ofstream fout(fn_out);
    fout<<"*Nodes"<<std::endl;
    for(size_t i=0; i<pts.size(); i++)
    {
        fout<< i+1 <<", " << pts[i].x <<", " << pts[i].y << ", " << pts[i].z << std::endl;
    }
    fout<<"*Elements"<<std::endl;
    int idx=1;
    for(auto& t: triangles)
    {
        if(t.size()!=3)
            throw std::runtime_error("if(t.size()!=3)");
        fout<< idx ;
        for(auto&z: t)
        {
            fout<< ", " << z+1;
        }
        fout << std::endl;
        idx++;
    }
    std::cout << "Print done" << std::endl;

}

void surface::load_points(const std::string& fn)
{
    auto b=loadFile(fn);

    auto ls=splitString("\r\n",b, 70000);
    for(auto& l:ls)
    {
        if(l.size() && l[0]=='*')
            continue;
        auto v=splitString(" ",l,4);
        if(v.size()==4)
        {
            Point p;
            int id=atoi(v[0].c_str());
            p.x=atof(v[1].c_str());
            p.y=atof(v[2].c_str());
            p.z=atof(v[3].c_str());
            pts.push_back(p);
        }
    }
    std::cout<< "Loaded pts " << pts.size() <<std::endl;

    return;
}

std::shared_ptr<Edge> surface::get_edge_or_create(const std::set<int>& s, const char *comment)
{

    auto i=all_edges.find(s);
    if(i==all_edges.end())
    {
        auto edge = std::make_shared<Edge>(s, comment);
        all_edges.insert({s,edge});

        return edge;
    }
    else {
        return i->second;

    }

}

void surface::calc_picture_size()
{
    Point min=pts[0];
    Point max=pts[0];

    for(size_t i=0; i<pts.size(); i++)
    {
        auto & p=pts[i];
        if(p.x>max.x) max.x=p.x;
        if(p.y>max.y) max.y=p.y;
        if(p.z>max.z) max.z=p.z;

        if(p.x<min.x) min.x=p.x;
        if(p.y<min.y) min.y=p.y;
        if(p.z<min.z) min.z=p.z;

    }

    Point d=max;
    d.sub(min);
    picture_size=std::max(d.x,std::max(d.y,d.z));
}

surface::~surface()
{
    /// ручная очистка нужна для разрешения коллизий с циклическими ссылками shared_ptr
    for(auto& p:pointInfos)
    {
        p.edges.clear();
        p.neighbours.clear();
        p.triangles.clear();
    }
    for(auto& e: all_edges)
    {
        e.second->opposize_pts.clear();
        e.second->points.clear();
    }
}

std::string loadFile(const std::string& name)
{
    std::ifstream t(name);
    t.seekg(0, std::ios::end);
    size_t size = t.tellg();
    std::string buffer(size, ' ');
    t.seekg(0);
    t.read(&buffer[0], size);
    return buffer;
}
std::vector<std::string> splitString(const char *seps, const std::string & src, size_t reserve)
{

    std::vector < std::string> res;
    res.reserve(reserve);
    std::set<char>mm;
    size_t l;
    l =::strlen(seps);
    for (unsigned int i = 0; i < l; i++)
    {
        mm.insert(seps[i]);
    }
    std::string tmp;
    l = src.size();

    for (unsigned int i = 0; i < l; i++)
    {

        if (!mm.count(src[i]))
            tmp += src[i];
        else
        {
            if (tmp.size())
            {
                res.push_back(tmp);
                tmp.clear();
            }
        }
    }

    if (tmp.size())
    {
        res.push_back(tmp);
        tmp.clear();
    }
    return res;
}
std::string dump_set_int(const std::set<int> &s)
{
    std::string out;
    for(auto& z:s)
    {
        out+=std::to_string(z)+" ";
    }
    return out;
}

int main(int argc, const char *argv[])
{

    std::string infile,outfile;
    for(int i=1; i<argc-1; i++)
    {
        if(std::string(argv[i])=="-i" ||(std::string)argv[i]=="--input")
            infile=argv[i+1];
        if((std::string)argv[i]=="-o" ||(std::string)argv[i]=="--output")
            outfile=argv[i+1];
    }
    if(infile.size()==0 || outfile.size()==0)
    {
        std::cout << "input/output files not specified, using default pathnames" << std::endl;
        /// по умолчанию используем фиксированные пути на диске для отладчика
        infile="/Users/sergejbelalov/saddle-nodes.xyz";
        outfile="saddle.xyz";
    }
    surface s;
    s.run(infile,outfile);

    return 0;
}
