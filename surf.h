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

    double figure_max_size;
    std::vector<point> reperz;

    struct _reperFind2
    {
        std::vector/*reper*/<std::vector<std::pair<double, int > > > distToPtsByReper;
//        std::vector<double> distForPt;
    };

    _reperFind2 reperFind2,reperFind2_copy;


    std::vector/*pt*/< std::set<int/*ref pt*/> > drawedNeighbours4Pt;

    std::set<int> find_1_NearestByReperz2(const point &pt, const std::set<int> &rebro, const std::set<int> &except_pts, int refcount);

    void load_points(const std::string& fn);
    void calculateReperz();
    void run(const std::string &fn, const std::string &fn_out);

};

#endif // SURF_H
