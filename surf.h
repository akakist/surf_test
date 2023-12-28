#ifndef SURF_H
#define SURF_H
#include "point.h"
#include <vector>
#include <set>
#include <map>
struct surface
{
    surface()
    {
    }
    std::vector<point> pts;

    std::vector<point> reperz;
    struct _reperFind
    {
        std::vector/*reper*/<std::map<long/*dist*/,std::set<int> > > distPtsToRepers;
        std::vector/*reper*/<std::vector/*pt*/<long/*dist*/> > distInRepersByPt;
    };
    _reperFind reperFind;

    std::vector/*pt*/< std::set<int/*ref pt*/> > drawedNeighbours4Pt;

    std::set<int> find_1_NearestByReperz(const point &pt, const std::set<int> &rebro, const std::set<int> &except_pts, int refcount);

    void removePointFromDistPtsToRepers(int pt);
    void load_points(const std::string& fn);
    void calculateReperz();
    void run(const std::string &fn, const std::string &fn_out);

};

#endif // SURF_H
