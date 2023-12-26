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
    std::vector/*reper*/<std::map<long/*dist*/,std::set<int> > > distPtsToRepers;
    std::vector/*reper*/<std::vector/*pt*/<long/*dist*/> > distInRepersByPt;

    int find_1_NearestByReperz(int pt, const std::set<int> &except_pts);

    void removePointFromDistPtsToRepers(int pt);
    void load_points(const std::string& fn);
    std::pair<double, std::set<int> > find_3_NearestPointsByReperz(int pt);
    void calculateReperz();
    void run(const std::string &fn, const std::string &fn_out);

};



#endif // SURF_H
