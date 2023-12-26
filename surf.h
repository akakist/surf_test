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
    /// tranlate pt id from sequential order to task order
    std::vector<int> ptIdx2TaskId;


    std::vector<point> reperz;
    std::vector/*reper*/<std::map<long/*dist*/,std::set<int> > > distPtsToRepers;
    std::vector/*reper*/<std::vector/*pt*/<long/*dist*/> > distInRepersByPt;
    std::set<int> ptsToSearch;// remained points which has < 3 neigbours

//    std::vector/*pt*/< std::set<int/*ref pt*/> > neighboursForPt; /// neigbours for each pt

//    void removePointFromDistPtsToRepers(int pt);
    void load_points(const std::string& fn);
    std::pair<double, std::set<int> > find3NearestByReperz(int pt);
    void calculateReperz();
    void run(const std::string &fn, const std::string &fn_out);

};



#endif // SURF_H
