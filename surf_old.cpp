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

void surface::step1_split_to_pairs()
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

            {

                REF_getter<rebro_container> rebro12=getRebroOrCreate({p1,p2},"first rebro");

                if(!rebro12.valid())
                    std::runtime_error("if(rebro12.valid())");
                _pi1.add_to_rebras(rebro12);
                _pi2.add_to_rebras(rebro12);

                _pi1.add_neighbours({p2});
                _pi2.add_neighbours({p1});


                REF_getter<figure> f=new figure(figIdGen++);
                f->points.insert({p1,p2});
                f->sum_points+=pts[p1]+pts[p2];

                f->rebras.insert({rebro12->points,rebro12});
                _pi1.figure_=f;
                _pi2.figure_=f;

                all_figures.insert({f->id,f});

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



