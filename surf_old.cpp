#include "surf.h"
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
    return pout;
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
