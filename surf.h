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
//    std::vector<std::map<long/*dist*/,std::set<int> > > distPtsToRepers_work;
    std::vector/*reper*/<std::vector/*pt*/<long/*dist*/> > distInRepersByPt;
    std::set<int> ptsToSearch;// remained points which has < 3 neigbours

    std::vector/*pt*/< std::set<int/*ref pt*/> > neighboursForPt; /// neigbours for each pt

    void removePointFromDistPtsToRepers_work(int pt);
    void load_points(const std::string& fn);
    int findNearestByReperz(int pt, const std::set<int> &except_pts);
    void calculateReperz();
    void run(const std::string &fn, const std::string &fn_out);

};



#endif // SURF_H
