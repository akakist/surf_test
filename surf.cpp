#include "surf.h"
#include "utils.h"
#include <fstream>
#include <iostream>
#include <deque>
#include <json/json.h>
#define MAX_N 20
point cross(const point& a,const point & v)
{
  return point(a.y*v.z - a.z*v.y, a.z*v.x - a.x*v.z, a.x*v.y - a.y*v.x);
}

bool surface::line_len_ok(real len)
{
//    return true;
    return len<picture_size/MAX_N;
}
bool is_triangle_good(const point& p1,const point&  p2, const point&  p3)
{
    return true;
    std::set<real> s;
    s.insert({fdist(p1,p2),fdist(p1,p3),fdist(p2,p3)});
    return *s.rbegin() / *s.begin()<1.3;
}

std::string dump_rebro(const std::set<int>& rebro)
{
    std::string out;
    out+= std::to_string(*rebro.begin());
    out+= " ";
    out+= std::to_string(*rebro.rbegin());
    return out;

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
REF_getter<triangle> surface::proceed_tiangle(int p0, int p2, int p3)
{
    auto &pi0=pointInfos[p0];
    auto &pi2=pointInfos[p2];
    auto &pi3=pointInfos[p3];
    auto r02=getRebroOrCreate({p0,p2},"link_neighbours2");
    auto r03=getRebroOrCreate({p0,p3},"link_neighbours2");
    auto r23=getRebroOrCreate({p2,p3},"link_neighbours2");

    if(r02->opposize_pts.size()>1 || r03->opposize_pts.size()>1 || r23->opposize_pts.size()>1)
    {
        throw std::runtime_error("if(r02->opposize_pts.size()>1 || r03->opposize_pts.size()>1 || r23->opposize_pts.size()>1)\n");

    }
    pi0.add_to_rebras(r02);
    pi0.add_to_rebras(r03);
    pi2.add_to_rebras(r02);
    pi2.add_to_rebras(r23);
    pi3.add_to_rebras(r03);
    pi3.add_to_rebras(r23);

    pi0.add_neighbours({p2,p3});
    pi2.add_neighbours({p0,p3});
    pi3.add_neighbours({p0,p2});
    r02->add_opposite_pts(p3);
    r03->add_opposite_pts(p2);
    r23->add_opposite_pts(p0);
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

    REF_getter<triangle> t=new triangle({p0,p2,p3});
    t->rebras.insert({r02,r03,r23});
    t->normal=cross(pts[p2]-pts[p0],pts[p3]-pts[p0]);
    all_triangles.insert({t->points,t});

    pi0.triangles.insert(t);
    pi2.triangles.insert(t);
    pi3.triangles.insert(t);
    if(!triangles.count({p0,p2,p3}))
    {
        triangles.insert({p0,p2,p3});
    }
    else
        throw std::runtime_error("2 triangle already inserted %s "+dump_set_int({p0,p2,p3}));

    return t;


}
bool surface::validate_triangle(int a, int b, int c)
{
    return true;
    auto d1=fdist(pts[a],pts[b]);
    auto d2=fdist(pts[a],pts[c]);
    auto d3=fdist(pts[b],pts[c]);
    auto max=std::max(d1, std::max(d2,d3));
    auto min=std::min(d1, std::max(d2,d3));
    if(max>min*10)
        return false;
    return true;

}
int surface::find_nearest(const point& pt, const std::set<int> &ps)
{
//    printf("find_nearest sz %d\n",ps.size());
    auto min=std::numeric_limits<real>::max();
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
int surface::find_nearest_which_can_be_added(const point& pt, const std::set<int> &unlinked_points, int p0,int p2, int p_opposite)
{
//    printf("find_nearest sz %d\n",ps.size());
    auto min=std::numeric_limits<real>::max();
    int sel=-1;
    for(auto& p:unlinked_points)
    {
        auto d=fdist(pt,pts[p]);
        if(d<min && line_len_ok(d) && triangle_can_be_added(p0,p2,p,p_opposite))
        {
            min=d;
            sel=p;
        }
    }
    return sel;
}

real surface::angle_between_3_points(int root,int a, int b)
{
    auto& p_root=pts[root];
    auto &p_a=pts[a];
    auto &p_b=pts[b];
    return Angle::angle(p_a-p_root,p_b-p_root);
}
point surface::cross_between_3_points(int root,int a, int b)
{
    auto& p_root=pts[root];
    auto &p_a=pts[a];
    auto &p_b=pts[b];
    return cross(p_a-p_root,p_b-p_root);
}
#define MAX_SUM_ANGLE_TRIANGLES 220
bool surface::triangle_can_be_added(int p0, int p2, int p_nearest, int p_opposite)
{

    /// вычисляем нормали к плоскостям {p2->p22_nearest,p2->p0} и к {p2->p0, p2->p_opp_r2}
    /// они сонаправлены и угол между ними должен быть < 90град

    auto normal2=cross_between_3_points(p2, p_nearest,p0);
    auto normalB2=cross_between_3_points(p2,p0,p_opposite);
    auto a=Angle::angle(normal2,normalB2);
    if(a<90)
    {
        return true;
    }
    return false;

}
void surface::proceed_on_angle_between_rebras(int p0, std::set<int>& unlinked_points,std::set<int>& active_points)
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
        real sum_angles=0;
        for(auto&t: pi0.triangles)
        {
//            printf("tri points %s\n", dump_set_int(t->points).c_str());
            auto ps=t->points;
            ps.erase(p0);
            if(ps.size()!=2)
                throw std::runtime_error("if(ps.size()!=2) "+dump_set_int(ps));

            int a=*ps.begin();
            int b=*ps.rbegin();
            sum_angles+=angle_between_3_points(p0,a,b);
        }
        if(sum_angles>MAX_SUM_ANGLE_TRIANGLES)
        {
            proceed_tiangle(p0,p2,p3);
            /// удаляем p0 из active_points поскольку она перестала быть граничной.
            active_points.erase(p0);
            return;
        }
        //// add unlinked point to both rebras
        auto center2=(pts[p0]+pts[p2])/2;
        auto center3=(pts[p0]+pts[p3])/2;
        if(r2->opposize_pts.size()!=1)
            throw std::runtime_error("if(r2->opposize_pts.size()!=1)");
        if(r3->opposize_pts.size()!=1)
            throw std::runtime_error("if(r2->opposize_pts.size()!=1)");
        int p_opp_r2=*r2->opposize_pts.begin();
        int p_opp_r3=*r2->opposize_pts.begin();

        auto p22_nearest=find_nearest_which_can_be_added(center2,unlinked_points,p0,p2,p_opp_r2);
        auto p33_nearest=find_nearest_which_can_be_added(center3,unlinked_points,p0,p3,p_opp_r3);


        /// если p22_nearest и p33_nearest совпадают, то строим все равно 2 треугольника и потом убираем ее из анлинкед поинтс
        ///
        /// берем точку, оппозитную ребру {p0,p2}, она одна, поскольку ребро не заполнено с обеих сторон
        if(p22_nearest!=-1)
        {


            {
                proceed_tiangle(p2,p0,p22_nearest);
                unlinked_points.erase(p22_nearest);
                active_points.insert(p22_nearest);
            }
        }

        if(p33_nearest!=-1){
            /// тоже самое с другой стороной
            if(r3->opposize_pts.size()!=1)
                throw std::runtime_error("if(r2->opposize_pts.size()!=1)");


            {
                proceed_tiangle(p3,p0,p33_nearest);
                unlinked_points.erase(p33_nearest);
                active_points.insert(p33_nearest);
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
}
void surface::flood()
{
    std::set<int> unlinked_points;
    std::set<int> active_points;
    std::set<REF_getter<triangle> > active_triangles;
    for(int i=0;i<pts.size();i++)
    {
        unlinked_points.insert(i);
    }
    /// make first triangle
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

    auto t=proceed_tiangle(p0,p2,p3);
    active_points.insert({p0,p2,p3});
    active_triangles.insert(t);
//////////////// end 1st triangle
    for(int i=0;i<200;i++)
    {
        auto a=active_points;
        for(auto& p:a)
        {
            proceed_on_angle_between_rebras(p,unlinked_points,active_points);
        }
    }



}
//    return n_result;
//}
void surface::run(const std::string &fn_in, const std::string& fn_out)
{

    std::srand(time(NULL));

    load_points(fn_in);

    std::cout << "Calculate triangles" <<std::endl;

    calc_picture_size();

    pointInfos.resize(pts.size());

    printf("max line %lf\n",picture_size/MAX_N);
    flood();
//    step1_split_to_pairs();
//    connect_unlinked_points();
//    supress_figures();
//    for(int i=0;i<50;i++)
//    {
//    for(auto& f:all_figures)
//        link_neighbours2(f.second);
//    }


    printf("all splitted to triangles\n");
//    pointInfo _pi1=pointInfos[p1];

    printf("triangles %d\n",triangles.size());
    printf("all rebvras %d\n",all_rebras.size());

    std::map<int,int> opps_for_rebra;
    std::map<const char*,int> byComments;
    for(auto&z: all_rebras)
    {
        opps_for_rebra[z.second->opposize_pts.size()]++;
        byComments[z.second->comment]++;
    }
    for(auto& z: opps_for_rebra)
    {
        printf("opps_for_rebra %d %d\n",z.first,z.second);
    }
    for(auto& z: byComments)
    {
        printf("byComments %s %d\n",z.first,z.second);
    }
//    for(auto& z: border_rebras)
//    {
//        printf("border_rebras %d %d\n",z.first,z.second);
//    }

    std::deque<std::set<int> > added_tri;
    /// connect points with 2 neigbours

    std::map<int,int> cnt;
    for(auto & z: pointInfos)
    {
        cnt[z.neighbours.size()]++;
    }
    for(auto &z: cnt)
    {
        std::cout << "by count of linked neigbours " << z.first  << " " << z.second << std::endl;
    }

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
            point p;
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

REF_getter<rebro_container> surface::getRebroOrCreate(const std::set<int>& s, const char *comment)
{
    point center= {0,0,0};
    for(auto& ss:s)
    {
        center+=pts[ss];
    }
    center/=s.size();

    auto i=all_rebras.find(s);
    if(i==all_rebras.end())
    {
        REF_getter<rebro_container >rebro = new rebro_container(s,center, comment);
        all_rebras.insert({s,rebro});

        return rebro;
    }
    else {
        return i->second;

    }

}

void surface::calc_picture_size()
{
    point min=pts[0];
    point max=pts[0];

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

    point d=max;
    d.sub(min);
    picture_size=std::max(d.x,std::max(d.y,d.z));
}
