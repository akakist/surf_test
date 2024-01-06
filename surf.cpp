#include "surf.h"
#include "utils.h"
#include <fstream>
#include <iostream>
#include <deque>
#include <json/json.h>
#define MAX_N 10
#define MIN_SUM_ANGLE_TRIANGLES 200
#define MAX_SUM_ANGLE_TRIANGLES (360-30)
#define MAX_ANGLE_BETWEEN_NORMALES 90

point cross(const point& a,const point & v)
{
    return point(a.y*v.z - a.z*v.y, a.z*v.x - a.x*v.z, a.x*v.y - a.y*v.x);
}

bool surface::line_len_ok(real len)
{
//    return true;
    return len<picture_size/MAX_N;
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
    auto d02=fdist(pts[p0],pts[p2]);
    auto d03=fdist(pts[p0],pts[p3]);
    auto d23=fdist(pts[p2],pts[p3]);
//    auto min=std::min(d02,std::min(d03,d23));
    auto max=std::max(d02,std::max(d03,d23));
    if(!line_len_ok(max))
        return nullptr;

    auto r02=getRebroOrCreate({p0,p2},"link_neighbours2");
    auto r03=getRebroOrCreate({p0,p3},"link_neighbours2");
    auto r23=getRebroOrCreate({p2,p3},"link_neighbours2");

    if(r02->opposize_pts.size()>1 || r03->opposize_pts.size()>1 || r23->opposize_pts.size()>1)
    {
        printf("r02 opp %d r03 opp %d r23 opp %d ",r02->opposize_pts.size(),r03->opposize_pts.size(),r23->opposize_pts.size());
        return nullptr;
        printf("r02 opp %d r03 opp %d r23 opp %d ",r02->opposize_pts.size(),r03->opposize_pts.size(),r23->opposize_pts.size());
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

    linked_points.insert({p0,p2,p3});

    return t;

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
int surface::find_nearest_which_can_be_added(const point& pt,  int p0,int p2, int p_opposite)
{
//    printf("find_nearest sz %d\n",ps.size());
    auto min=std::numeric_limits<real>::max();
    int sel_unlinked=-1;
    for(auto& p:unlinked_points)
    {
        auto d=fdist(pt,pts[p]);
//        bool

        if(d<min && line_len_ok(d) && triangle_can_be_added(p0,p2,p,p_opposite))
        {
            min=d;
            sel_unlinked=p;
        }
    }
#ifdef KALL
    if(sel_unlinked!=-1)
    {
        if(min>picture_size/(MAX_N*3))
        {
            min=std::numeric_limits<real>::max();
            int sel_linked=-1;
            for(auto& p:linked_points)
            {
                auto d=fdist(pts[sel_unlinked],pts[p]);
                if(d<min)
                {
                    sel_linked=p;
                    min=d;
                }

            }
            if(sel_linked!=-1)
            {
                auto pi_linked=pointInfos[sel_linked];
                auto z=pi_linked.not_filles_rebras();
                auto rs=pi_linked.rebras;
                if(rs.size() && z.size()==0)
                {
                    unlinked_points.erase(sel_linked);
                    return -1;
                }
            }
        }
    }
#endif
    return sel_unlinked;

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
bool surface::triangle_can_be_added(int p0, int p2, int p_nearest, int p_opposite)
{

//    return true;
    /// вычисляем нормали к плоскостям {p2->p22_nearest,p2->p0} и к {p2->p0, p2->p_opp_r2}
    /// они сонаправлены и угол между ними должен быть < 90град

//    return true;
    auto normal2=cross_between_3_points(p2, p_nearest,p0);
    auto normalB2=cross_between_3_points(p2,p0,p_opposite);
    auto a=Angle::angle(normal2,normalB2);
    if(a<MAX_ANGLE_BETWEEN_NORMALES)
    {
        return true;
    }
    return false;

}
int surface::proceed_connection_between_tops(int p0)
{
    printf("proceed_connection_between_tops\n");
    auto& pi0=pointInfos[p0];
    /// берем ребра, у которых треугольник есть только с одной стороны
    auto notfilled=pi0.not_filles_rebras();

    if(notfilled.size()==2)
    {
        printf("if(notfilled.size()==2)\n");
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
        printf("sum angles %lf\n",sum_angles);
        if(sum_angles>MIN_SUM_ANGLE_TRIANGLES /*&& sum_angles<MAX_SUM_ANGLE_TRIANGLES*/)
        {
            printf("try proceed_tiangle\n");
            auto t=proceed_tiangle(p0,p2,p3);
            /// удаляем p0 из active_points поскольку она перестала быть граничной.
            if(t.valid())
            {
                printf("ok\n");
                active_points.erase(p0);

                return 1;
            }
        }
        else
        {
            printf("cancel due angle\n");
        }

    }
    return 0;

}

void surface::proceed_add_new_point_between_rebras(int p0)
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

#ifdef KALL
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
        if(sum_angles>MIN_SUM_ANGLE_TRIANGLES && sum_angles<MAX_SUM_ANGLE_TRIANGLES)
        {
            auto t=proceed_tiangle(p0,p2,p3);
            /// удаляем p0 из active_points поскольку она перестала быть граничной.
            if(t.valid())
            {
                active_points.erase(p0);
                return;
            }
        }
#endif
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
            if(proceed_tiangle(p2,p0,p22_nearest).valid())
            {
                unlinked_points.erase(p22_nearest);
                active_points.insert(p22_nearest);
            }
        }

        if(p33_nearest!=-1) {
            /// тоже самое с другой стороной
            if(proceed_tiangle(p3,p0,p33_nearest).valid())
            {
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
void surface::recalc_active_points()
{
    std::set<int> s;
    for(int i=0; i<pointInfos.size(); i++)
    {
        if(pointInfos[i].not_filles_rebras().size()==2)
            s.insert(i);
    }
    if(active_points!=s)
        printf("active_points != s\n");
    active_points=s;
}
void surface::flood()
{
    for(int i=0; i<pts.size(); i++)
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
    if(t.valid())
    {
        active_points.insert({p0,p2,p3});
    }

    //////////////// end 1st triangle
    for(int i=0; i<450; i++)
    {
//        recalc_active_points();
        auto a=active_points;
        for(auto& p:a)
        {
//            printf("step %d\n",p);
            proceed_add_new_point_between_rebras(p);
            do
            {
            }
            while(proceed_connection_between_tops(p)!=0);

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

    printf("all splitted to triangles\n");

    printf("triangles %d\n",triangles.size());
    printf("all rebras %d\n",all_rebras.size());

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
