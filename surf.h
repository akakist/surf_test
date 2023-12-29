#ifndef SURF_H
#define SURF_H
#include "point.h"
#include <vector>
#include <set>
#include <map>
#include "REF.h"
#include "utils.h"
#include <deque>
//struct point_p;

struct rebro_container:public Refcountable
{
    std::set<int> points;
    std::set<int> opposize_pts;
    rebro_container(){}
    rebro_container(const std::set<int>&s):points(s){}

};
struct pointInfo
{
    std::deque<REF_getter<rebro_container>> rebras;
    std::set<int> neighbours;
};


struct surface
{
    surface()
    {
    }
    std::vector<point > pts;


    struct _algoFind
    {
        _algoFind():need_rebuild(true){}
        std::multimap<real,int> dists;
        point reper;
        bool need_rebuild;
        void rebuild(const point &p, const std::vector<point >&pts)
        {
            dists.clear();
            reper=p;
            for(int i=0;i<pts.size();i++)
            {
                dists.insert({fdist(p,pts[i]),i});
            }
        }
        int findNearest(const point &pt, const std::vector<point >&pts,const std::vector/*pt*/< std::set<int/*ref pt*/> >& drawedNeighbours4Pt,const std::set<int> &rebro, const std::set<int> &except_pts, int refcount)
        {
            if(need_rebuild)
            {
                rebuild(pt,pts);
                need_rebuild=false;
            }
            real reper_dist=fdist(pt,reper);
            std::set<int> result;
            for(auto it=dists.begin();it!=dists.end();it++)
            {
                auto& newp=it->second;
                auto& d=it->first;
                if(d>reper_dist && result.size())
                    break;
                {
                    if(drawedNeighbours4Pt[newp].size()>refcount)
                    {
                        continue;
                    }
                    if(rebro.count(newp))
                    {
                        continue;
                    }
                    if(except_pts.count(newp))
                    {
                        continue;
                    }


                    result.insert(newp);
                }
            }
            real min_d=std::numeric_limits<double>::max();
            int selected=-1;
            for(auto& z: result)
            {
                double d=fdist(pt,pts[z]);
                if(d<min_d)
                {
                    min_d=d;
                    selected=z;
                }

            }
            if(result.size()>1000)
                need_rebuild=true;

            printf("selected %d\n",selected);
            return selected;

        }
    };

    _algoFind algoFind;

    std::vector/*pt*/< std::set<int/*ref pt*/> > drawedNeighbours4Pt;
    std::vector<pointInfo> pointInfos;

    int found_cnt=0;
    int not_found_cnt=0;
    int result_empty=0;

    point rebro_center(const REF_getter<rebro_container> & rebro);


    void load_points(const std::string& fn);
    void calculateReperz();
    void run(const std::string &fn, const std::string &fn_out);

};

#endif // SURF_H
