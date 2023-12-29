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
struct ptsInfo
{
    std::deque<REF_getter<rebro_container>> rebras;
};


struct surface
{
    surface()
    {
    }
    std::vector<point > pts;

    std::vector<point> reperz;
    struct _reperFind
    {
        std::vector/*reper*/<std::map<long/*dist*/,std::set<int> > > distPtsToRepers;
        std::vector/*reper*/<std::vector/*pt*/<long/*dist*/> > distInRepersByPt;
    };
    _reperFind reperFind;
    _reperFind reperFind_backup;

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
//                return dists.begin()->second;
            }
            real rdist=fdist(pt,reper);
            std::set<int> result;
            for(auto &z: dists)
            {
                if(z.first<rdist)
                {
                    if(drawedNeighbours4Pt[z.first].size()>refcount)
                        continue;
                    if(rebro.count(z.first))
                        continue;
                    if(except_pts.count(z.first))
                        continue;

                    result.insert(z.second);
                }
                else break;
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

            return selected;

        }
    };

    _algoFind algoFind;

    std::vector/*pt*/< std::set<int/*ref pt*/> > drawedNeighbours4Pt;

    int found_cnt=0;
    int not_found_cnt=0;
    int result_empty=0;

    point rebro_center(const REF_getter<rebro_container> & rebro);

    std::set<int> find_1_NearestByReperz(const point &pt, const std::set<int> &rebro, const std::set<int> &except_pts, int refcount);

    void removePointFromDistPtsToRepers(const std::set<int> &s);
    void load_points(const std::string& fn);
    void calculateReperz();
    void run(const std::string &fn, const std::string &fn_out);

};

#endif // SURF_H
