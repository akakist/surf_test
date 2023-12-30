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
    const char *comment=nullptr;
    rebro_container(){}
    rebro_container(const std::set<int>&s, const char* comm):points(s), comment(comm){
        if(s.size()!=2)
            throw std::runtime_error("if(s.size()!=2)");
    }


};
struct pointInfo
{
    std::map<std::set<int>,REF_getter<rebro_container>> rebras;
    std::set<int> neighbours;
    bool arounded=false;
    void add_neighbours(const std::set<int> &s)
    {
        for(auto& z: s)
            neighbours.insert(z);
    }
    void calcArounded()
    {
        if(arounded)
            return;
        bool result=true;
        for(auto&r: rebras)
        {
            if(r.second->opposize_pts.size()!=2)
            {
                result=false;
                break;
            }

        }
        if(result){
//            printf("arounded true\n");
            arounded=result;
        }
    }
    std::map<std::set<int>, REF_getter<rebro_container> > getUnfilledRebras();
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

    point rebro_center(const REF_getter<rebro_container> & rebro);

    std::set<int> get_rebro_neighbours(const REF_getter<rebro_container>&r);


    void load_points(const std::string& fn);
    void run(const std::string &fn, const std::string &fn_out);
    void process_rebras(std::set<int> &searchSet, int refcount, bool append_new_rebras);

    int find(const point &pt, const std::set<int> &rebro, const std::set<int> &except_pts, int refcount)
    {
        return algoFind__findBrutforce(pt,rebro,except_pts,refcount);
    }

    REF_getter<rebro_container> getRebroOrCreate(const std::set<int>& s, const char* comment);
    void process_point(const std::vector<int> vp);

    void calc_figure_size();

    std::set<int> get_rebro_pts(const REF_getter<rebro_container>&r)
    {
        std::set<int> s;
        for(auto& p:r->points)
        {
            s.insert(p);
            pointInfo &pi=pointInfos[p];
            for(auto& n:pi.neighbours)
            {
                s.insert(n);
            }
        }
        for(auto& o:r->opposize_pts)
        {
            s.insert(o);
        }
        return s;
    }

};

#endif // SURF_H
