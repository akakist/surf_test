#ifndef SURF_H
#define SURF_H
#include "point.h"
#include <vector>
#include <set>
#include <map>
#include "REF.h"
#include "utils.h"
#include <deque>
#include <string.h>
//struct point_p;
enum ABLE {
    ABLE_FIND_AND_ADD=1,
    ABLE_CONNECT_NEIGHBOUR=2,
    ABLE_SKIP=3
};
#define _FL_ (std::string)" "+  __FILE__ +" "+std::to_string(__LINE__)

struct rebro_container:public Refcountable
{
    std::set<int> points;
    std::set<int> opposize_pts;
    const char *comment=nullptr;
    void add_opposite_pts(int p)
    {
        opposize_pts.insert(p);
        if(opposize_pts.size()>2)
        {
            throw std::runtime_error("if(opposize_pts.size()>2)" + _FL_.c_str());
        }
    }
    int left()
    {
        if(points.size()!=2)
            throw std::runtime_error("if(points.size()!=2)"+ _FL_.c_str());
        return *points.begin();
    }
    int right()
    {
        if(points.size()!=2)
            throw std::runtime_error("if(points.size()!=2)"+ _FL_.c_str());
        return *points.rbegin();
    }
    rebro_container() {}
    rebro_container(const std::set<int>&s, const char* comm):points(s), comment(comm) {
        if(s.size()!=2)
            throw std::runtime_error("if(s.size()!=2)");
    }

};
struct pointInfo
{
    std::map<std::set<int>,REF_getter<rebro_container>> rebras;

    std::set<int> neighbours;

    std::set<REF_getter<rebro_container>> border_rebras_get()
    {
        std::set<REF_getter<rebro_container>> ret;
        for(auto& r:rebras)
        {
            if(r.second->opposize_pts.size()==1)
            {
                ret.insert(r.second);
            }
        }
        return ret;

    }
    void add_to_rebras(const REF_getter<rebro_container>& r)
    {
        rebras.insert({r->points,r});
    }
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
        _algoFind():need_rebuild(true) {}
        std::multimap<real,int> dists;
        point reper;
        bool need_rebuild;
    };
    int algoFind__findBrutforce(const point &pt, const std::set<int> &rebro, const std::set<int> &except_pts, int refcount);
    int algoFind__findNearest(const point &pt,
                              const std::set<int> &rebro, const std::set<int> &except_pts, int refcount);
    void algoFind__rebuild(const point &p, const std::vector<point >&pts, const std::set<int> &searchSet);

    _algoFind algoFind;

    std::vector<pointInfo> pointInfos;
    std::map<std::set<int>,REF_getter<rebro_container> > rebras_to_process;
    std::map<std::set<int>,REF_getter<rebro_container> > all_rebras;
    std::set<std::set<int> > triangles;
    std::set<int> searchSet;
    real figure_size;

    std::deque<REF_getter<rebro_container>> border_rebras_to_process;

    point rebro_center(const REF_getter<rebro_container> & rebro);
    int get_rebro_peer(const REF_getter<rebro_container> &rebro, int n);

    std::set<int> get_rebro_neighbours(const REF_getter<rebro_container>&r);

    void load_points(const std::string& fn);
    void run(const std::string &fn, const std::string &fn_out);

    int find(const point &pt, const std::set<int> &rebro, const std::set<int> &except_pts, int refcount)
    {
        return algoFind__findBrutforce(pt,rebro,except_pts,refcount);
    }

    REF_getter<rebro_container> getRebroOrCreate(const std::set<int>& s, const char* comment);
    void process_point(int p1);

    void find_and_add_point_to_rebro(const REF_getter<rebro_container>& rebro12);

    void calc_figure_size();
    ABLE can_find_and_add_new(int pt);


};

#endif // SURF_H
