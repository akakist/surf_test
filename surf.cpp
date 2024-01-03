#include "surf.h"
#include "utils.h"
#include <fstream>
#include <iostream>
#include <deque>
#include <json/json.h>
#define MAX_N 20
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

int surface::get_rebro_peer(const REF_getter<rebro_container>& rebro,int n)
{

    if(rebro->points.size()!=2)
        throw std::runtime_error("if(rebro.size()!=2)");
    if(!rebro->points.count(n))
        throw std::runtime_error("if(!rebro.count(n))");
    for(auto& z:rebro->points)
    {
        if(z!=n)
            return z;
    }
    throw std::runtime_error("not found");
}
point surface::rebro_center(const REF_getter<rebro_container> & rebro)
{
    point p;
    for(auto& s: rebro->points)
    {
        p.add(pts[s]);
    }
    p.div(rebro->points.size());
    return p;
}
struct compare_stuff
{
    REF_getter<rebro_container> r2;
    REF_getter<rebro_container> r3;
    int peerpoint2,peerpoint3;
    compare_stuff():r2(nullptr),r3(nullptr) {}
};
//std::deque<REF_getter<figure> >
//std::map<int,REF_getter<figure> > figures;
#ifdef KALL
void surface::step4_process_points()
{
    int n=0;
    for(auto&f: all_figures)
    {
        for(auto &p: f.second->points)
        {
            n=process_point(p,f.second);
        }
    }
//    for(int i=0; i<pts.size(); i++)
//    {
//        n+=process_point(i);
//    }
    printf("step4_process_points %d tri inserted\n",n);
}
#endif
void surface::move_content(const REF_getter<figure>& to, const REF_getter<figure>&from)
{
    for(auto&p: from->points)
    {
        to->points.insert(p);
        to->sum_points+=pts[p];
        pointInfos[p].figure_=to;
    }
    for(auto& r: from->rebras)
    {
        to->rebras.insert({r.first,r.second});
    }
    from->points.clear();
    from->sum_points= {0,0,0};
    from->rebras.clear();
    all_figures.erase(from->id);

}
std::string surface::dump_figure(const REF_getter<figure>&f)
{
    Json::Value j;
//    f->points;
    std::map<int, int> bord;
    j["pcount"]=(int)f->points.size();
    for(auto& p:f->points)
    {
        auto& pi=pointInfos[p];
        bord[pi.can_connected()]++;
    }
    for(auto& z:bord)
    {
        j["bord"][std::to_string(z.first)]=z.second;
    }
    return j.toStyledString();
//    f->
}
bool surface::figure_has_unbordered(const REF_getter<figure>& f)
{
    for(auto& p:f->points)
    {
        pointInfo& pi=pointInfos[p];
        if(pi.can_connected())
        {

        }

    }
}
void surface::make_rebring3()
{
    std::set<int> searchSet;
    for(int i=0; i<pts.size(); i++)
    {
        searchSet.insert(i);
    }

}
void surface::make_rebring2()
{
    std::set<int> searchSet;
    for(int i=0; i<pts.size(); i++)
    {
        searchSet.insert(i);
    }
    std::set<int> processedSet;
    processedSet.insert(0);
    int p0=0;
    searchSet.erase(p0);
//    std::set<int> processed;
    while(searchSet.size())
    {
        auto min=std::numeric_limits<real>::max();
        int selected=-1;
        for(auto& s:searchSet)
        {
            auto d=fdist(pts[p0],pts[s]);
            if(d<min && line_len_ok(d))
            {
                selected=s;
                min=d;
            }
        }
        if(processedSet.count(selected))
        {
            printf("processedSet.count(selected)\n");
        }
        if(selected==-1 /*|| processedSet.count(selected)*/)
        {
            break;
#ifdef KALL
            printf("start new branch\n");
            printf("if(selected==-1 || processedSet.count(selected))\n");
            for(int i=0; i<pts.size(); i++)
            {
                searchSet.insert(i);
            }

            int sel=-1;
            int cnt=0;

            for(int z=0; z<pointInfos.size(); z++)
            {
//                searchSet.clear();
                if(pointInfos[z].neighbours.size()==0)
                    sel=z;
            }
            printf("new brach start %d\n",sel);
            if(sel==-1)
                break;
            searchSet.erase(sel);
            p0=sel;
            //printf("cnt=%d\n",cnt);
            //throw std::runtime_error("if(selected==-1)");
            continue;
#endif
        }
        auto &pi0=pointInfos[p0];
        auto &pi1=pointInfos[selected];
        pi0.add_neighbours({selected});
        pi1.add_neighbours({p0});
        auto r=getRebroOrCreate({p0,selected},"make_rebring");
        pi0.add_to_rebras(r);
        pi1.add_to_rebras(r);
        searchSet.erase(selected);
        p0=selected;
//        processedSet.insert(p0);
        processedSet.insert(selected);
    }
    for(int i=0; i<pts.size(); i++)
    {
        searchSet.insert(i);
    }

    std::set<int> lost;
    for(int z=0; z<pointInfos.size(); z++)
    {
//                searchSet.clear();
        if(pointInfos[z].neighbours.size()<2)
            lost.insert(z);
    }
    for(auto& l:lost)
    {
        printf("lost %d\n",l);
        auto &pil=pointInfos[l];
        auto min=std::numeric_limits<real>::max();
        int selected=-1;
        for(auto& s:searchSet)
        {
            if(s==l) continue;
            if(pil.neighbours.count(s))
                continue;
            auto d=fdist(pts[l],pts[s]);
            if(d<min && line_len_ok(d))
            {
                selected=s;
                min=d;
            }
        }
        if(selected==-1)
        {
            break;
            throw std::runtime_error("if(selected==-1)");
        }

        int p0=l;
        auto &pi0=pointInfos[p0];
        auto &pi1=pointInfos[selected];
        pi0.add_neighbours({selected});
        pi1.add_neighbours({p0});
        auto r=getRebroOrCreate({p0,selected},"lost");
        pi0.add_to_rebras(r);
        pi1.add_to_rebras(r);
        searchSet.erase(selected);
        p0=selected;

    }

}
void surface::connect_unlinked_points()
{
    std::set<int> to_process;
    std::set<int> seachSet;
    for(int i=0;i<pointInfos.size();i++)
    {
        auto &pi=pointInfos[i];
        if(pi.neighbours.size()==0 || pi.neighbours.size()==1)
        {
            to_process.insert(i);
        }
        else
        {
            seachSet.insert(i);
        }
    }
//    for(auto& c: to_process)
    {
        for(auto& p: to_process)
        {
            auto min=std::numeric_limits<real>::max();
            int selected=-1;
            for(auto& s:seachSet)
            {
                if(s==p) continue;
    //            if(pil.neighbours.count(s))
    //                continue;
                auto d=fdist(pts[p],pts[s]);
                if(d<min /*&& line_len_ok(d)*/)
                {
                    selected=s;
                    min=d;
                }
            }
            auto r=getRebroOrCreate({p,selected},"connect_unlinked");
            auto p0=p;
            auto p1=selected;
            auto &pi0=pointInfos[p];
            auto &pi1=pointInfos[selected];
            pi0.add_neighbours({p1});
            pi1.add_neighbours({p0});
            pi0.add_to_rebras(r);
            pi1.add_to_rebras(r);
            if(!pi1.figure_.valid())
                throw std::runtime_error("if(!pi1.figure_.valid())");
            pi0.figure_=pi1.figure_;
            pi1.figure_->points.insert(p0);
            pi1.figure_->sum_points+=pts[p];



        }
    }
}
void surface::supress_figures()
{
    std::multimap<int, REF_getter<figure> > sorted_f;
    for(auto& f: all_figures)
    {
        sorted_f.insert({f.second->points.size(),f.second});
    }
    for(auto&f: sorted_f)
    {
        if(f.second->points.size()==0)
            continue;
        auto min=std::numeric_limits<real>::max();
        int selected=-1;
        point p0=f.second->center_point();
        for(int i=0;i<pts.size();i++)
        {
            auto &pi=pointInfos[i];
            if(pi.figure_->id==f.second->id)
                continue;
            auto d=fdist(pts[i],p0);
            if(d<min)
            {
                selected=i;
                min=d;
            }
        }
        if(selected==-1)
            continue;
        auto &pisel=pointInfos[selected];
        auto f2=pisel.figure_;
        if(f.second->points.size()==0)
            throw std::runtime_error("if(f.second->points.size()==0)");
        if(f2.valid() && f.second.valid())
        {
            connect_2_figures(f2,f.second);
            triangulate_figure(f2);
        }
    }

}
REF_getter<figure> surface::connect_2_figures(const REF_getter<figure>& f1, const REF_getter<figure>& f2)
{
    if(f1->points.size()==0)
        throw std::runtime_error("if(f1->points.size()==0)");
    int n2=get_nearest_point_in_figure(f2,f1->center_point());
    int n1=get_nearest_point_in_figure(f1,pts[n2]);
    if(f1->points.size()>3)
    {
        n2=get_nearest_point_in_figure(f2,pts[n1]);
    }
    if(f2->points.size()>3)
    {
        n1=get_nearest_point_in_figure(f1,pts[n2]);
    }
    auto r=getRebroOrCreate({n1,n2},"connect_2_figures");
    auto &pi1=pointInfos[n1];
    auto &pi2=pointInfos[n2];
    pi1.add_neighbours({n2});
    pi2.add_neighbours({n1});
    pi1.add_to_rebras(r);
    pi2.add_to_rebras(r);
    move_content(f1,f2);
    return f1;
}

void surface::triangulate_figure(const REF_getter<figure> &F)
{
    auto ps=F->points;
    for(auto& p:ps)
    {
        auto& pi=pointInfos[p];
        if(!pi.can_connected())
        {
            F->points.erase(p);
            F->sum_points-=pts[p];
        }
        else
        {
            std::set<int> interesting_points;
            process_point2(p,interesting_points);
        }
    }
    for(auto&p: F->points)
    {
        auto& pi=pointInfos[p];
        if(!pi.can_connected())
            throw std::runtime_error("if(!pi.can_connected())");
    }
}
//void surface::/*connect_2_figures*/(
void surface::process_point2(int p0, std::set<int>& interesting_points)
{
//    while(interested_points.size())
    {
//        printf("interested_points.size() %d\n",interested_points.size());
//        int p0=*interested_points.begin();
//        interested_points.erase(interested_points.begin());
        /// 2 кейса - первый - 2 ребра с одним оппозитом
        /// либо 2 ребра с одним оппозитом и 1 ребро без оппозитов.
        /// в первом случае соединяем 2 ребра если угол < 150 и он вогнутый.
        /// внутреннюю точку удаляем из фигуры и делаем ее is_boarded=false
        /// во втором случае соединяем 2 пары, если угол <150

        auto& pi0=pointInfos[p0];
        printf("process_point2 pi0.rebras.size() %d\n",pi0.rebras.size());
        if(pi0.rebras.size()==0) return;
        if(pi0.rebras.size()==1) return;
//        if(pi0.rebras.size()==2)
        {
            if(pi0.rebras.size()==2)
            {
                printf("if(pi0.rebras.size()==2)\n");
                printf("p0 %d\n",p0);
                std::vector<int> tops;
                for(auto& r:pi0.rebras)
                    printf("r opps %d\n",r.second->opposize_pts.size());

                printf("pi0 can connected %d\n",pi0.can_connected());
                for(auto& z:pi0.rebras)
                {
                    auto s=z.second->points;
                    s.erase(p0);
                    if(s.size()!=1)
                        throw std::runtime_error("if(s.size()!=1)");

                    tops.push_back(*s.begin());
                }
                if(tops.size()==2)
                {
                    printf("if(tops.size()==2)\n");
                    int p2=*tops.begin();
                    int p3=*tops.rbegin();
                    auto a=Angle::angle(pts[p2]-pts[p0],pts[p3]-pts[p0]);
                    printf("angle %lf\n",a);
                    if(a<90)
                    {
                        auto r02=getRebroOrCreate({p0,p2},"wrong case");
                        auto r03=getRebroOrCreate({p0,p3},"wrong case");
                        printf("join zzz p0,p2,p3 %d %d %d\n",p0,p2,p3);
                        printf("rebro exists {%d,%d} %d\n",p2,p3,all_rebras.count({p2,p3}));
                        auto r23=getRebroOrCreate({p2,p3},"process_point2-1");
                        auto &pi0=pointInfos[p0];
                        auto &pi2=pointInfos[p2];
                        auto &pi3=pointInfos[p3];
                        pi2.add_neighbours({p3});
                        pi3.add_neighbours({p2});
                        pi2.add_to_rebras(r23);
                        pi3.add_to_rebras(r23);
                        r23->opposize_pts.insert(p0);
                        r02->opposize_pts.insert(p3);
                        r03->opposize_pts.insert(p2);
printf("triangles count %d\n",triangles.count({p0,p2,p3}));
                        triangles.insert({p0,p2,p3});
                        interesting_points.insert({p0,p2,p3});

                    }

                }

            }
            return;
        }

        {
            printf("rebras %d\n",pi0.rebras.size());
            std::vector<int> tops;
            for(auto& z:pi0.rebras)
            {
                auto s=z.second->points;
                s.erase(p0);
                if(s.size()!=1)
                    throw std::runtime_error("if(s.size()!=1)");

                tops.push_back(*s.begin());
            }
            std::vector<std::set<int>> all_combinations;
            for(int i=0; i<tops.size(); i++)
            {
                for(int j=0; j<tops.size(); j++)
                {
                    if(tops[i]!=tops[j])
                    {
                        all_combinations.push_back({tops[i],tops[j]});
                    }
                }
            }
            /// вычисляем углы для каждой комбинации
            std::map<real, std::set<int>  > angle_for_combination;
            for(auto& c:all_combinations)
            {
                auto p2=*c.begin();
                auto p3=*c.rbegin();
                auto a=Angle::angle(pts[p2]-pts[p0],pts[p3]-pts[p0]);
                printf("a %lf\n",a);
                angle_for_combination.insert({a,c});
            }
            if(angle_for_combination.size()==0)
                throw std::runtime_error("if(angle_for_combination.size()==0)");

            /// находим по 2 комбинации с наименьшим углом для каждой вершины.
//                sort(angle_for_combination.begin(),angle_for_combination.end());
            std::map<int,std::set<std::set<int>>> collections;
//                for(auto& top: tops)
            {
                for(auto& c: angle_for_combination)
                {
                    auto p2=*c.second.begin();
                    auto p3=*c.second.rbegin();
                    if(collections[p2].size()<2)
                    {
                        collections[p2].insert(c.second);
                    }
                    if(collections[p3].size()<2)
                    {
                        collections[p3].insert(c.second);
                    }
                }
            }
            /// собираем пары в одну кучу
            std::set<std::set<int>> heap;
            for(auto& z: collections)
            {
                for(auto& s:z.second)
                {
                    heap.insert(s);
                }
            }
            /// соединяем пары
            for(auto&c: heap)
            {

                auto p2=*c.begin();
                auto p3=*c.rbegin();
                auto a=Angle::angle(pts[p2]-pts[p0],pts[p3]-pts[p0]);
                if(a>120)
                {
                    printf("if(a>160) \n");
                    continue;
                }
                auto r02=getRebroOrCreate({p0,p2},"wrong case");
                auto r03=getRebroOrCreate({p0,p3},"wrong case");
                if(r02->opposize_pts.size()==2)
                    continue;
                if(r03->opposize_pts.size()==2)
                    continue;
                printf("join zzz p0,p2,p3 %d %d %d\n",p0,p2,p3);
                auto r23=getRebroOrCreate({p2,p3},"process_point2");
                auto &pi0=pointInfos[p0];
                auto &pi2=pointInfos[p2];
                auto &pi3=pointInfos[p3];
                pi2.add_neighbours({p3});
                pi3.add_neighbours({p2});
                pi2.add_to_rebras(r23);
                pi3.add_to_rebras(r23);
                r23->opposize_pts.insert(p0);
                r02->opposize_pts.insert(p3);
                r03->opposize_pts.insert(p2);
                triangles.insert({p0,p2,p3});
                interesting_points.insert({p0,p2,p3});

            }

        }

//            auto r0=pi0.border_rebras_get(0);
//            auto r1=pi0.border_rebras_get(1);
//            printf("r0 size %d\n",r0.size());
//            printf("r1 size %d\n",r1.size());
//            if(r1.size()==2)
//            {

//            }
    }

}

void surface::fill_rebring_2()
{
    std::set<int> interesting;
    for(int i=0; i<pts.size(); i++)
    {
        process_point2(i,interesting);
    }
    while(interesting.size())
    {
        auto cp=interesting;
        interesting.clear();
        for(auto& p:interesting)
        {
            process_point2(p,interesting);
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
    int p1=0;
    std::set<int> s1;
    s1.insert(p1);
    printf("p1 %d\n",p1);

    step1_split_to_rectangles();
    connect_unlinked_points();
    for(auto&f: all_figures)
        triangulate_figure(f.second);
    supress_figures();

    for(int i=0;i<10;i++)
    {
        for(auto&f: all_figures)
            triangulate_figure(f.second);
    }
printf("figure_count %d\n",all_figures.size());

//    stepN_create_figures_from_unlinked_points();
//    make_rebring2();
//    fill_rebring_2();
//    fill_rebring_2();
//    fill_rebring_2();
//    fill_rebring_2();
//    printf
#ifdef KALL
    stepN_create_figures_from_unlinked_points();
//    int n=step2_connect_unlinked_points();
//    printf("step2_connect_unlinked_points %d\n",n);
    printf("step3_connect_figures %d\n",step3_connect_figures());
//    step4_process_points();
//    step4_process_points();
    printf("step3_connect_figures %d\n",step3_connect_figures());
//    step4_process_points();
    printf("step3_connect_figures %d\n",step3_connect_figures());
    for(int i=0; i<10; i++)
    {
        printf("step3_connect_figures %d\n",step3_connect_figures());
//        step4_process_points();
    }
#endif
    printf("all splitted to triangles\n");
//    return;
    pointInfo _pi1=pointInfos[p1];
//    auto p2=find(searchSet,pts[p1], {p1}, {},0);
//    printf("p2 %d\n",p2);

    printf("triangles %d\n",triangles.size());
    printf("all rebvras %d\n",all_rebras.size());
    std::map<int,int> border_rebras;
    for(int i=0; i<pointInfos.size(); i++)
    {
        pointInfo& pi=pointInfos[i];
        border_rebras[pi.can_connected()]++;
    }

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
    for(auto& z: border_rebras)
    {
        printf("border_rebras %d %d\n",z.first,z.second);
    }

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
int surface::algoFind__findBrutforce(const std::set<int>& searchSet1,
                                     const point &pt,  const std::set<int> &rebro, const std::set<int> &except_pts, int refcount)
{
    real min_d=std::numeric_limits<double>::max();
    int selected=-1;
    for(auto &i:searchSet1)
    {

        pointInfo pi=pointInfos[i];

        if(pi.neighbours.size()>refcount)
        {
            continue;
        }

        if(rebro.count(i))
        {
            continue;
        }
        if(except_pts.count(i))
        {
            continue;
        }

        double d=fdist(pt,pts[i]);
        if(! line_len_ok(d))
            continue;
        if(d<min_d)
        {

            min_d=d;

            selected=i;
        }

    }
    return selected;
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
