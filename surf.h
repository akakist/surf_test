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
    std::map<std::set<int>,REF_getter<rebro_container>> rebras;
    std::set<int> neighbours;
    void add_neighbours(const std::set<int> &s)
    {
        for(auto& z: s)
            neighbours.insert(z);
    }
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
        int findBrutforce(const point &pt, const std::vector<point >&pts,const std::vector<pointInfo>& pointInfos,const std::set<int> &rebro, const std::set<int> &except_pts, int refcount)
        {
            real min_d=std::numeric_limits<double>::max();
            int selected=-1;
            for(int i=0;i<pts.size();i++)
            {
                if(pointInfos[i].neighbours.size()>refcount)
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

                if(d<min_d)
                {


                    min_d=d;

                    selected=i;
                }

            }
            return selected;
        }
        int findNearest(const point &pt, const std::vector<point >&pts,const std::vector<pointInfo>& pointInfos,const std::set<int> &rebro, const std::set<int> &except_pts, int refcount)
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
                    if(pointInfos[newp].neighbours.size()>refcount)
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

    std::vector<pointInfo> pointInfos;
    std::deque<REF_getter<rebro_container> > rebras_to_process;
    std::map<std::set<int>,REF_getter<rebro_container> > all_rebras;
    std::set<std::set<int> > triangles;


    point rebro_center(const REF_getter<rebro_container> & rebro);


    void load_points(const std::string& fn);
    void run(const std::string &fn, const std::string &fn_out);
    void process_rebras(int refcount, bool append_new_rebras);

    int find(const point &pt, const std::set<int> &rebro, const std::set<int> &except_pts, int refcount)
    {
        return algoFind.findBrutforce(pt,pts,pointInfos,rebro,except_pts,refcount);
    }

};

#endif // SURF_H
