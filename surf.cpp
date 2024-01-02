#include "surf.h"
#include "utils.h"
#include <fstream>
#include <iostream>
#include <deque>
#include <json/json.h>
#define MAX_N 10
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
#ifdef KALL
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
    std::map<real,compare_stuff> to_compare;
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
//            std::map<real,std::set<>>
            {
                /// unlinked
                compare_stuff cs;
                cs.peerpoint2=peerpoint2;
                cs.peerpoint3=peerpoint3;
                cs.r2=r2;
                cs.r3=r3;
                to_compare[Angle::angle(pts[peerpoint2]-pts[p1],pts[peerpoint3]-pts[p1])]=cs;

//                printf("unlinked\n");
//                unlinked_pairs.push_back({r2,r3});

            }

        }
//        printf("unlinked_pairs %d\n",unlinked_pairs.size());
    }
    if(to_compare.size())
    {
        compare_stuff cs=to_compare.begin()->second;
        if(is_triangle_good(pts[p1],pts[cs.peerpoint2],pts[cs.peerpoint3]))
        {
            ret_N_inserted_rebras++;
            pointInfo &pi1=pointInfos[p1];
            pointInfo &pi2=pointInfos[cs.peerpoint2];
            pointInfo &pi3=pointInfos[cs.peerpoint3];

            REF_getter<rebro_container> rebro23(getRebroOrCreate({cs.peerpoint2,cs.peerpoint3},"process rebras"));
            REF_getter<rebro_container> rebro13(cs.r3);
            REF_getter<rebro_container> rebro12(cs.r2);

            if(rebro12->opposize_pts.size()<2 &&
                    rebro13->opposize_pts.size()<2 &&
                    rebro23->opposize_pts.size()<2 )
            {

                pi2.add_neighbours({cs.peerpoint3});
                pi3.add_neighbours({cs.peerpoint2});

                pi2.add_to_rebras(rebro23);
                pi3.add_to_rebras(rebro23);
                fig->rebras.insert({rebro23->points,rebro23});

                rebro12->add_opposite_pts(cs.peerpoint3);
                rebro13->add_opposite_pts(cs.peerpoint2);
                rebro23->add_opposite_pts(p1);
                triangles.insert({p1,cs.peerpoint2,cs.peerpoint3});
            }
            else
                printf("skipped by all has 2 opps\n");

        }

    }

    return ret_N_inserted_rebras;

}
#endif
//std::deque<REF_getter<figure> >
//std::map<int,REF_getter<figure> > figures;
#ifdef KALL
int surface::step2_connect_unlinked_points()
{
    int result_cnt=0;
    {
        std::map<int,REF_getter<figure>> figures_to_process=all_figures;

        /// connect triangles to nearest figure
        std::set<int> unlinked_pts;
        for(int i=0; i<pointInfos.size(); i++)
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
//                    REF_getter<rebro_container> sel_rebro_peer(nullptr);
                    int sel_pt=-1;
                    auto min=std::numeric_limits<real>::max();
                    for(auto& p:f_peer->points)
                    {
                        pointInfo &pi=pointInfos[p];
                        if(pi.border_rebras_get().size() || pi.neighbours.size()<3)
                        {
                            auto d=fdist(pts[p1], pts[p]);
                            if(d<min && line_len_ok(d))
                            {
                                min=d;
                                sel_pt=p;
                            }

                        }
                    }
                    if(sel_pt!=-1)
                    {
//                        auto p2=sel_rebro_peer->left();
//                        auto p3=sel_rebro_peer->right();
//                        auto A2=abs(60-Angle::angle(pts[p2],pts[p1]));
//                        auto A3=abs(60-Angle::angle(pts[p3],pts[p1]));
//                        int p23;
//                        REF_getter<rebro_container>r(nullptr);
//                        if(A2<A3)
//                        {
//                            p23=2;
//                        }
//                        else
//                        {
//                            p23=3;
//                        }
                        auto r=getRebroOrCreate({p1,sel_pt},"connecting");
                        auto &pi1=pointInfos[p1];
                        auto &pisel_pt=pointInfos[sel_pt];

                        pisel_pt.add_to_rebras(r);
                        pi1.add_to_rebras(r);
                        pisel_pt.add_neighbours({p1});
                        pi1.add_neighbours({sel_pt});

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
#endif
void surface::stepN_create_figures_from_unlinked_points()
{
    int cnt=0;
    for(int i=0; i<pts.size(); i++)
    {
        auto& pi=pointInfos[i];
        if(pi.neighbours.size()==0)
        {
//            pi.is_bordered=true;
            REF_getter<figure> f=new figure(figIdGen++);
            f->points.insert(i);
            f->sum_points+=pts[i];
            all_figures.insert({f->id,f});

            cnt++;

        }
    }
    printf("stepN_create_figures_from_unlinked_points %d\n",cnt);
}
void surface::step1_split_to_rectangles()
{
    std::set<int>searchSet;
    for(int i=0; i<pts.size(); i++)
    {
        searchSet.insert(i);
    }

    std::set<int> except;

//    std::map<int,std::set<REF_getter<figure>>> figures;
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
//                _pi1.is_bordered=true;
//                _pi2.is_bordered=true;
//                _pi3.is_bordered=true;
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
//    printf("searchset size %d\n",searchSet.size());

}
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
    from->sum_points={0,0,0};
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
void surface::make_rebring2()
{
    std::set<int> searchSet;
    for(int i=0;i<pts.size();i++)
    {
        searchSet.insert(i);
    }
    std::set<int> processedSet;
    processedSet.insert(0);
    int p0=0;
    searchSet.erase(p0);
    std::deque<int> processed;
    while(searchSet.size())
    {
        auto min=std::numeric_limits<real>::max();
        int selected=-1;
        for(auto& s:searchSet)
        {
            auto d=fdist(pts[p0],pts[s]);
            if(d<min)
            {
                selected=s;
                min=d;
            }
        }
        if(selected==-1)
        {
            throw std::runtime_error("if(selected==1)");
        }
        auto pi0=pointInfos[p0];
        auto pi1=pointInfos[selected];
        pi0.add_neighbours({selected});
        pi1.add_neighbours({p0});
        auto r=getRebroOrCreate({p0,selected},"make_rebring");
        pi0.add_to_rebras(r);
        pi1.add_to_rebras(r);
        searchSet.erase(selected);
        p0=selected;
    }
}
int surface::get_nearest_point_in_figure(const REF_getter<figure>& f1,const point& p)
{
    auto min=std::numeric_limits<real>::max();
    int pout=-1;
    for(auto& p1: f1->points)
    {
//        auto &pi=pointInfos[p1];
        {
            auto d=fdist(pts[p1], p);
            if(d<min /*&& line_len_ok(d)*/)
            {
                min=d;
                pout=p1;
            }
        }
    }
    if(pout==-1)
    {
        throw std::runtime_error("if(pout==-1)");
    }
}
REF_getter<figure> surface::connect_2_figures(const REF_getter<figure>& f1, const REF_getter<figure>& f2)
{

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
void surface::fill_rebring_1()
{
    for(int i=0;i<pts.size();i++)
    {
        int p0=i;
        pointInfo & pi0=pointInfos[i];
        if(pi0.rebras.size()==2)
        {
            std::vector<int> tops;
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
                int p2=*tops.begin();
                int p3=*tops.rbegin();
                auto a=Angle::angle(pts[p2]-pts[p0],pts[p3]-pts[p0]);
                if(a<90)
                {

                }

            }
            std::vector<std::set<int>> all_combinations;
            for(int i=0;i<tops.size();i++)
            {
                for(int j=0;j<tops.size();j++)
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
//                    printf("a %lf\n",a);
                angle_for_combination.insert({a,c});
            }

        }
    }
}
void surface::make_rebring()
{
    while(all_figures.size()>1)
    {
        printf("all_figures %d\n",all_figures.size());
        std::map<int,REF_getter<figure>> figures_to_process=all_figures;
        std::vector<std::pair<REF_getter<figure>, REF_getter<figure> > > fi_pairs;

        while(figures_to_process.size())
        {
            REF_getter<figure> f_process=figures_to_process.begin()->second;
            REF_getter<figure> f_peer(nullptr);
            printf("f_process %d\n",f_process->id);
            auto min1=std::numeric_limits<real>::max();
            for(auto& f_all:all_figures)
            {

                if(f_process->id!=f_all.second->id)
                {
                    auto d=fdist(f_process->center_point(),f_all.second->center_point());
                    if(d<min1 /*&& line_len_ok(d)*/)
                    {
                        min1=d;
                        f_peer=f_all.second;
                    }
                }
            }

            if(!f_peer.valid())
            {
                printf("if(!f_peer.valid())\n")                ;
//               figures_result.insert({f_process->id,f_process});
               continue;
            }
            printf("f_process %d\n",f_process->id);
            printf("f_peer %d\n",f_peer->id);


            fi_pairs.push_back({f_process,f_peer});
            figures_to_process.erase(f_process->id);
            figures_to_process.erase(f_peer->id);


        }
        for(auto& peer: fi_pairs)
        {
//            REF_getter<figure> f_result(nullptr);
            std::set<int> interested_points;
            auto f_process=peer.first;
            auto f_peer=peer.second;
            auto f=connect_2_figures(peer.first,peer.second);
//            figures_result.insert({f->id,f});
        }
    }
}
int surface::step3_connect_figures()
{
    std::string bug_report;
    int n_result=0;
    std::map<int,REF_getter<figure>> figures_to_process=all_figures;
    printf("figures_to_process %d\n",figures_to_process.size());
    /// connect triangles to nearest figure
    std::vector<std::pair<REF_getter<figure>, REF_getter<figure> > > fi_pairs;
    std::map<int,REF_getter<figure>> figures_result;
    REF_getter<figure> f_process=figures_to_process.begin()->second;
    REF_getter<figure> f_peer(nullptr);
    REF_getter<figure> f_result(nullptr);

    std::set<int> interested_points;
    while(figures_to_process.size())
    {
        REF_getter<figure> f_process=figures_to_process.begin()->second;
        figures_to_process.erase(f_process->id);

        if(f_process->points.size()==0)
            throw std::runtime_error("if(f_process->points.size()==0)");
        REF_getter<figure> f_peer(nullptr);
        {
            auto min1=std::numeric_limits<real>::max();
            for(auto& f_all:all_figures)
            {

                if(f_process->id!=f_all.second->id)
                {
                    auto d=fdist(f_process->center_point(),f_all.second->center_point());
                    if(d<min1 /*&& line_len_ok(d)*/)
                    {
                        min1=d;
                        f_peer=f_all.second;
                    }
                }
            }
        }
        if(!f_peer.valid())
        {
           figures_result.insert({f_process->id,f_process});
           continue;
        }


        fi_pairs.push_back({f_process,f_peer});
        if(f_process.valid())
            figures_to_process.erase(f_process->id);
        if(f_peer.valid())
            figures_to_process.erase(f_peer->id);
    }
    for(auto& peer: fi_pairs)
    {
        std::set<int> interested_points;
        auto f_process=peer.first;
        auto f_peer=peer.second;
        auto f=connect_2_figures(peer.first,peer.second);
        figures_result.insert({f->id,f});
    }



//    REF_getter<figure> f_process=figures_to_process.begin()->second;
//    REF_getter<figure> f_peer(nullptr);

        if(!f_process.valid() || !f_peer.valid())
            throw std::runtime_error("if(!f_process.valid() || !f_peer.valid())");
//        if(f_peer.valid())
        {
//            printf("f_peer valid %d\n",f_peer.valid());

            /// находим сначала ближайшую точку из f_process к фигуре f_peer
            /// затем находим ближайшее ребро из f_peer

            auto min=std::numeric_limits<real>::max();
            int p_process=-1;
            for(auto& p: f_process->points)
            {
                auto &pi=pointInfos[p];
                if(pi.can_connected())
                {
                    auto d=fdist(pts[p], f_peer->center_point());
                    if(d<min /*&& line_len_ok(d)*/)
                    {
                        min=d;
                        p_process=p;
                    }
                }
            }
            if(p_process==-1)
            {
//                continue;
                bug_report+="if(!p_in_process.valid())\n";

                bug_report+="all_figures.size() "+std::to_string(all_figures.size())+"\n";
//                printf("all_figures.size() %d\n",all_figures.size());
//                printf("f_process->points size %d\n",f_process->points.size());
                for(auto& z: f_process->points)
                {
                    bug_report+=pointInfos[z].dump(z);
                }
//                continue;
                printf("bug %s\n",bug_report.c_str());
                printf("f_process %s\n",dump_figure(f_process).c_str());
                throw std::runtime_error("if(p_in_process==-1)");
            }

//                REF_getter<rebro_container> sel_rebro_peer(nullptr);
            int p_in_peers=-1;
            min=std::numeric_limits<real>::max();
            for(auto& p:f_peer->points)
            {
                auto &pi=pointInfos[p];
                if(pi.can_connected())
                {
                    auto d=fdist(pts[p_process], pts[p]);
                    if(d<min /*&& line_len_ok(d)*/)
                    {
                        min=d;
                        p_in_peers=p;
                    }
                }
            }
            if(p_in_peers==-1)
                throw std::runtime_error("if(selected_pt_in_peers==-1)");

            interested_points.insert({p_process,p_in_peers});
            auto &pi_in_peers=pointInfos[p_in_peers];
            auto &pi_in_process=pointInfos[p_process];
//            pi_in_peers.is_bordered=true;
//            pi_in_process.is_bordered=true;

            auto rebro=getRebroOrCreate({p_process,p_in_peers},"connect");

            pi_in_peers.add_to_rebras(rebro);
            pi_in_process.add_to_rebras(rebro);
            pi_in_peers.add_neighbours({p_process});
            pi_in_process.add_neighbours({p_in_peers});

            n_result++;

            if(f_peer->points.size()>f_process->points.size())
            {
                move_content(f_peer,f_process);
//                f_result=f_peer;

            } else
            {
                move_content(f_process,f_peer);
//                f_result=f_process;
            }

//                process_point(p_in_process,f_peer);

        }
        std::set<int> processed_points;
        while(interested_points.size())
        {
            printf("interested_points.size() %d\n",interested_points.size());
            int p0=*interested_points.begin();
            interested_points.erase(interested_points.begin());
            /// 2 кейса - первый - 2 ребра с одним оппозитом
            /// либо 2 ребра с одним оппозитом и 1 ребро без оппозитов.
            /// в первом случае соединяем 2 ребра если угол < 150 и он вогнутый.
            /// внутреннюю точку удаляем из фигуры и делаем ее is_boarded=false
            /// во втором случае соединяем 2 пары, если угол <150

            auto& pi0=pointInfos[p0];
            if(pi0.rebras.size()<3)
                continue;
            if(processed_points.count(p0))
                continue;

            processed_points.insert(p0);
            {
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
                for(int i=0;i<tops.size();i++)
                {
                    for(int j=0;j<tops.size();j++)
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
//                    printf("a %lf\n",a);
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
                    if(a>160)
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
                    auto r23=getRebroOrCreate({p2,p3},"connecting");
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
//                    pi0.is_bordered=pi0.border_rebras_count();
//                    pi2.is_bordered=pi2.border_rebras_count();
//                    pi3.is_bordered=pi3.border_rebras_count();
//                    printf("interested add %d %d\n",p2,p3);
//                    printf("all figs %d\n",all_figures.size());
//                    pi2.dump(p2);
//                    pi3.dump(p3);
                    interested_points.insert(p2);
                    interested_points.insert(p3);
                    if(!pi0.can_connected())
                    {
                        f_result->points.erase(p0);
                        f_result->sum_points-=pts[p0];
                    }
                    if(!pi2.can_connected())
                    {
                        f_result->points.erase(p2);
                        f_result->sum_points-=pts[p2];
                    }
                    if(!pi3.can_connected())
                    {
                        f_result->points.erase(p3);
                        f_result->sum_points-=pts[p3];
                    }
                    triangles.insert({p0,p2,p3});

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
return n_result;
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

//    stepN_create_figures_from_unlinked_points();
    make_rebring2();
//    printf
#ifdef KALL
    step1_split_to_rectangles();
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
