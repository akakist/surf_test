#ifndef SURF_H
#define SURF_H
#include "point.h"
#include <vector>
#include <set>
#include <map>
#include "REF.h"
//struct point_p;

struct rebro_container:public Refcountable
{
    std::set<int> points;
    std::set<int> opposize_pts;
    rebro_container(){}
    rebro_container(const std::set<int>&s):points(s){}

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
