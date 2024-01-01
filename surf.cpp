#include "surf.h"
#include "utils.h"
#include <fstream>
#include <iostream>
#include <deque>
#include <json/json.h>
#define MAX_N 3
bool surface::line_len_ok(real len)
{
    return len<picture_size/20;
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
int surface::process_point( int p1, const REF_getter<figure>& fig)
{
    int ret_N_inserted_rebras=0;
    pointInfo &pi1=pointInfos[p1];
    std::set<std::set<REF_getter<rebro_container>>> border_rebras_pairs;

    {
//        if(pi1.rebras.size()!=3)return ret_N_inserted_rebras;
        auto border_rebras1=pi1.border_rebras_get();

        /// берем все комбинации ребер, выходящих из одной точки
            for(auto& r1: border_rebras1)
            {
                for(auto& r2: border_rebras1)
                {

                    if(r2->points!=r1->points)
                    {
//                        printf("border_rebras_pairs.insert({r1,r2});\n");
                        border_rebras_pairs.insert({r1,r2});
                    }
                    else
                    {
//                        printf("!border_rebras_pairs.insert({r1,r2});\n");

                    }
                }
            }
    }
//    printf("border_rebras_pairs size %d\n",border_rebras_pairs.size());
    /// вычисляем пары ребер, которые соединены перекладиной
    /// и выкидываем их из списка border_rebras_pairs

    std::vector<std::set<REF_getter<rebro_container> > > unlinked_pairs;
    {
        for(auto& br: border_rebras_pairs)
        {
            auto r3=*br.begin();
            auto r2=*br.rbegin();
            auto peerpoint3=get_rebro_peer(r3,p1);
            auto peerpoint2=get_rebro_peer(r2,p1);
//            printf("p1 peerpoint2 peerpoint3 %d %d %d\n",p1,peerpoint2,peerpoint3);
            auto &pi2=pointInfos[peerpoint2];
            auto &pi3=pointInfos[peerpoint3];
            if(pi2.neighbours.count(peerpoint3)
                    &&  pi3.neighbours.count(peerpoint2))
            {
                /// linked
//                printf("linked\n");
            }
            {
                /// unlinked
//                printf("unlinked\n");
//                unlinked_pairs.push_back({r2,r3});
                if(is_triangle_good(pts[p1],pts[peerpoint2],pts[peerpoint3]))
                {
                    ret_N_inserted_rebras++;
                    pointInfo &pi1=pointInfos[p1];
                    pointInfo &pi2=pointInfos[peerpoint2];
                    pointInfo &pi3=pointInfos[peerpoint3];
                    REF_getter<rebro_container> rebro23(getRebroOrCreate({peerpoint2,peerpoint3},"process rebras"));
                    REF_getter<rebro_container> rebro13(r3);
                    REF_getter<rebro_container> rebro12(r2);

                    if(rebro12->opposize_pts.size()<2 &&
                            rebro13->opposize_pts.size()<2 &&
                            rebro23->opposize_pts.size()<2 )
                    {

                        pi2.add_neighbours({peerpoint3});
                        pi3.add_neighbours({peerpoint2});


                        pi2.add_to_rebras(rebro23);
                        pi3.add_to_rebras(rebro23);
                        fig->rebras.insert({rebro23->points,rebro23});

                        rebro12->add_opposite_pts(peerpoint3);
                        rebro13->add_opposite_pts(peerpoint2);
                        rebro23->add_opposite_pts(p1);
                        triangles.insert({p1,peerpoint2,peerpoint3});
                    }
                    else
                        printf("skipped by all has 2 opps\n");

                }

            }

        }
//        printf("unlinked_pairs %d\n",unlinked_pairs.size());
    }
    return ret_N_inserted_rebras;


}

//std::deque<REF_getter<figure> >
std::map<int,REF_getter<figure> > figures;
int figIdGen=0;

int surface::step2_connect_unlinked_points()
{
    int result_cnt=0;
    {
        std::map<int,REF_getter<figure>> figures_to_process=all_figures;

        /// connect triangles to nearest figure
        std::set<int> unlinked_pts;
        for(int i=0;i<pointInfos.size();i++)
        {
            if(pointInfos[i].neighbours.empty())
                unlinked_pts.insert(i);
        }

        while(unlinked_pts.size())
        {
            int p1=*unlinked_pts.begin();
            unlinked_pts.erase(unlinked_pts.begin());

            REF_getter<figure> f_peer(nullptr);
            {
                auto min1=std::numeric_limits<real>::max();
                for(auto& f_all:all_figures)
                {

                            auto d=fdist(pts[p1],f_all.second->center_point());
                            if(d<min1 && line_len_ok(d))
                            {
                                min1=d;
                                f_peer=f_all.second;
                            }
                }
            }
            if(f_peer.valid())
            {
                {
                    REF_getter<rebro_container> sel_rebro_peer(nullptr);
                    auto min=std::numeric_limits<real>::max();
                    for(auto& r:f_peer->rebras)
                    {
                        if(r.second->opposize_pts.size()==2)
                            continue;
                        auto d=fdist(pts[p1], r.second->center);
                        if(d<min && line_len_ok(d))
                        {
                            min=d;
                            sel_rebro_peer=r.second;
                        }
                    }
                    if(sel_rebro_peer.valid())
                    {
                        auto p2=sel_rebro_peer->left();
                        auto p3=sel_rebro_peer->right();
                        auto A2=abs(60-Angle::angle(pts[p2],pts[p1]));
                        auto A3=abs(60-Angle::angle(pts[p3],pts[p1]));
                        int p23;
                        REF_getter<rebro_container>r(nullptr);
                        if(A2<A3)
                        {
                            p23=2;
                        }
                        else
                        {
                            p23=3;
                        }
                        r=getRebroOrCreate({p1,p23},"connecting");
                        auto &pi1=pointInfos[p1];
                        auto &pi23=pointInfos[p23];


                        pi23.add_to_rebras(r);
                        pi1.add_to_rebras(r);
                        pi23.add_neighbours({p1});
                        pi1.add_neighbours({p23});


                        result_cnt++;
                        f_peer->rebras.insert({r->points,r});
                        f_peer->points.insert(p1);
                        f_peer->sum_points+=pts[p1];
                    }
                    process_point(p1,f_peer);

                }
            }

        }

    }
    return result_cnt;

}
void surface::step1_split_to_rectangles()
{
    std::set<int>searchSet;
    for(int i=0; i<pts.size(); i++)
    {
        searchSet.insert(i);
    }

    std::set<int> except;

    std::map<int,std::set<REF_getter<figure>>> figures;
    while(searchSet.size())
    {
        int p1=*searchSet.begin();

        pointInfo &_pi1=pointInfos[p1];
        except.insert(p1);
        int p2=find(searchSet,pts[p1], {}, except,0);
        if(p2!=-1)
        {
            except.insert(p2);
            pointInfo &_pi2=pointInfos[p2];

            int p3=find(searchSet,(pts[p2]+pts[p1])/2, {}, except,0);
            if(p3!=-1)
            {
                pointInfo &_pi3=pointInfos[p3];
                except.insert(p3);

                REF_getter<rebro_container> rebro12=getRebroOrCreate({p1,p2},"first rebro");
                REF_getter<rebro_container> rebro13=getRebroOrCreate({p1,p3},"first rebro");
                REF_getter<rebro_container> rebro23=getRebroOrCreate({p2,p3},"first rebro");

                if(!rebro12.valid())
                    std::runtime_error("if(rebro12.valid())");
                if(!rebro13.valid())
                    std::runtime_error("if(rebro13.valid())");
                if(!rebro23.valid())
                    std::runtime_error("if(rebro23.valid())");
                _pi1.add_to_rebras(rebro12);
                _pi1.add_to_rebras(rebro13);

                _pi2.add_to_rebras(rebro12);
                _pi2.add_to_rebras(rebro23);

                _pi3.add_to_rebras(rebro13);
                _pi3.add_to_rebras(rebro23);

                _pi1.add_neighbours({p2,p3});
                _pi2.add_neighbours({p1,p3});
                _pi3.add_neighbours({p1,p2});

                rebro23->add_opposite_pts(p1);
                rebro13->add_opposite_pts(p2);
                rebro12->add_opposite_pts(p3);
                REF_getter<figure> f=new figure(figIdGen++);
                f->points.insert({p1,p2,p3});
                f->sum_points+=pts[p1]+pts[p2]+pts[p3];

                f->rebras.insert({rebro12->points,rebro12});
                f->rebras.insert({rebro13->points,rebro13});
                f->rebras.insert({rebro23->points,rebro23});
                _pi1.figure_=f;
                _pi2.figure_=f;
                _pi3.figure_=f;

                all_figures.insert({f->id,f});

                triangles.insert({p1,p2,p3});
            }

        }

        for(auto& e:except)
        {
            searchSet.erase(e);
        }

    }
///Users/sergejbelalov/surf_test/surf_test/surf.cpp
    except.clear();

}
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


int surface::step3_connect_figures()
{
    int n_result=0;
    std::map<int,REF_getter<figure>> figures_to_process=all_figures;
    printf("figures_to_process %d\n",figures_to_process.size());
    /// connect triangles to nearest figure

    while(figures_to_process.size())
    {
        REF_getter<figure> f_process=figures_to_process.begin()->second;
        figures_to_process.erase(f_process->id);

        REF_getter<figure> f_peer(nullptr);
        {
            auto min1=std::numeric_limits<real>::max();
            for(auto& f_all:all_figures)
            {

                {
                    if(f_process->id!=f_all.second->id)
                    {
                        auto d=fdist(f_process->center_point(),f_all.second->center_point());
                        if(d<min1 && line_len_ok(d))
                        {
                            min1=d;
                            f_peer=f_all.second;
                        }
                    }
                }
            }
//            if(found)
//                printf("f_peer dist %lf\n",min1);
//            printf("f_peer valid %d\n",f_peer.valid());
        }
        if(f_peer.valid())
        {
            printf("f_peer valid %d\n",f_peer.valid());
            std::set<std::set<int>> rm;
            for(auto& z: f_peer->rebras)
            {
                if(z.second->opposize_pts.size()==2)
                    rm.insert(z.second->points);

            }
    //        for(auto& z: rm)
    //        {
    //            f_peer->rebras.erase(z);
    //        }
            if(f_peer->rebras.size()==rm.size())
                printf("if(f_peer->rebras.size()==rm.size())\n");

            /// находим сначала ближайшую точку из f_process к фигуре f_peer
            /// затем находим ближайшее ребро из f_peer

            auto min=std::numeric_limits<real>::max();
            int sel_point_process=-1;
            for(auto& p: f_process->points)
            {
                auto d=fdist(pts[p], f_peer->center_point());
                if(d<min && line_len_ok(d))
                {
                    min=d;
                    sel_point_process=p;
                }
            }
            if(sel_point_process==-1)
                throw std::runtime_error("if(sel_p==-1)");
//            printf("sel_point_process %d\n",sel_point_process);
            /// find nearest rebro from f_peer to point sel_point_process

            {
                REF_getter<rebro_container> sel_rebro_peer(nullptr);
                auto min=std::numeric_limits<real>::max();
                for(auto& r:f_peer->rebras)
                {
//                    printf("r.second->opposize_pts.size() %d\n",r.second->opposize_pts.size());
                    if(r.second->opposize_pts.size()==2)
                        continue;
                    auto d=fdist(pts[sel_point_process], r.second->center);
                    if(d<min && line_len_ok(d))
                    {
                        min=d;
                        sel_rebro_peer=r.second;
                    }
                }
                if(sel_rebro_peer.valid())
                {
//                    printf("if(sel_rebro_peer.valid())\n");
                    n_result++;
                    auto p1=sel_rebro_peer->left();
                    auto p2=sel_rebro_peer->right();
                    auto p3=sel_point_process;
                    auto A1=abs(60-Angle::angle(pts[p1],pts[p3]));
                    auto A2=abs(60-Angle::angle(pts[p2],pts[p3]));
                    int selp;
                    if(A1<A2)
                    {
                        selp=p1;
                    }
                    else
                    {
                        selp=p2;
                    }
                    auto r=getRebroOrCreate({p3,selp},"connecting");
                    auto &pi3=pointInfos[p3];
                    auto &selpi=pointInfos[selp];


                    selpi.add_to_rebras(r);
                    pi3.add_to_rebras(r);
                    selpi.add_neighbours({p3});
                    pi3.add_neighbours({selp});
//                    triangles.insert({p1,p2,p3});




                    for(auto&p: f_process->points)
                    {

                        f_peer->points.insert(p);
                        f_peer->sum_points+=pts[p];
                        pointInfos[p].figure_=f_peer;
                    }
                    for(auto& r: f_process->rebras)
                    {
                        if(r.second->opposize_pts.size()!=2)
                            f_peer->rebras.insert({r.first,r.second});
                    }
                    f_process->rebras.clear();
                    f_process->points.clear();
                    f_process->sum_points= {0,0,0};
                    all_figures.erase(f_process->id);
                }
                process_point(sel_point_process,f_peer);

            }
        }

    }
    return n_result;
}
void surface::run(const std::string &fn_in, const std::string& fn_out)
{

    std::srand(time(NULL));

    load_points(fn_in);

    std::cout << "Calculate triangles" <<std::endl;

    calc_picture_size();

    pointInfos.resize(pts.size());

    int p1=0;
    std::set<int> s1;
    s1.insert(p1);
    printf("p1 %d\n",p1);

    step1_split_to_rectangles();
    int n=step2_connect_unlinked_points();
    printf("step2_connect_unlinked_points %d\n",n);
    printf("step3_connect_figures %d\n",step3_connect_figures());
    step4_process_points();
    step4_process_points();
    printf("step3_connect_figures %d\n",step3_connect_figures());
    step4_process_points();
    printf("step3_connect_figures %d\n",step3_connect_figures());
    for(int i=0;i<10;i++)
    step4_process_points();

    printf("all splitted to triangles\n");
//    return;
    pointInfo _pi1=pointInfos[p1];
//    auto p2=find(searchSet,pts[p1], {p1}, {},0);
//    printf("p2 %d\n",p2);


    printf("triangles %d\n",triangles.size());
    printf("all rebvras %d\n",all_rebras.size());
    std::map<int,int> border_rebras;
    for(int i=0;i<pointInfos.size();i++)
    {
        pointInfo& pi=pointInfos[i];
        border_rebras[pi.border_rebras_get().size()]++;
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
