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
#include <json/json.h>
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
    point center;
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
    rebro_container(const std::set<int>&s,const point& center_, const char* comm):points(s), comment(comm), center(center_) {
        if(s.size()!=2)
            throw std::runtime_error("if(s.size()!=2)");

    }

};
struct triangle: public Refcountable
{
    int id;
    std::set<int> points;
    point normal;
    std::set<REF_getter<rebro_container> > rebras;
    std::set<REF_getter<triangle>> neigbours;
    triangle(const std::set<int> & pt)
    {
        if(pt.size()!=3)
            throw std::runtime_error("if(pt.size()!=3)");
        points=pt;
    }
    std::set<REF_getter<rebro_container>> active_borders()
    {
        std::set<REF_getter<rebro_container>> ret;
        for(auto& r: rebras)
        {
            if(r->opposize_pts.size()==1)
                ret.insert(r);
            return ret;
        }

    }
};
struct figure: public Refcountable
{
    figure(int _id):id(_id) {}
    int id;
    std::map<std::set<int>,REF_getter<rebro_container> > rebras;
    std::set<int> points;
    point sum_points= {0,0,0};
    point center_point()
    {
        return sum_points/points.size();
    }

};

struct pointInfo
{
    pointInfo():figure_(nullptr) {}
    std::map<std::set<int>,REF_getter<rebro_container>> rebras;

    std::set<REF_getter<triangle>> triangles;

    std::set<int> neighbours;
    REF_getter<figure> figure_;
//    bool is_bordered=false;
    std::string dump(int pt)
    {
        Json::Value j;
        j["pt"]=pt;
//        j["is_bordered"]=is_bordered;
        j["figure_"]=figure_->id;
        for(auto& n:neighbours)
        {
            j["neighbours"].append(n);
        }
        return j.toStyledString()+"\n";
    }
    std::set<REF_getter<rebro_container> > not_filles_rebras()
    {
        std::set<REF_getter<rebro_container> > ret;
        for(auto& r:rebras)
        {
            if(r.second->opposize_pts.size()<2)
            {
                ret.insert(r.second);
            }
        }
        return ret;
    }
    bool can_connected()
    {
        if(rebras.size()==0)
            return true;
        if(rebras.size()==1)
        {
            return true;
        }
        for(auto& r:rebras)
        {
            if(r.second->opposize_pts.size()<2)
            {
                return true;
            }
        }
        return false;

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

    int algoFind__findBrutforce(const std::set<int> &searchSet1, const point &pt, const std::set<int> &rebro, const std::set<int> &except_pts, int refcount);

    std::vector<pointInfo> pointInfos;
    std::map<std::set<int>,REF_getter<rebro_container> > rebras_to_process;
    std::map<std::set<int>,REF_getter<rebro_container> > all_rebras;
    std::set<std::set<int> > triangles;
    std::set<int> searchSetG;
    real picture_size;
    int step3_connect_figures();
//    void step4_process_points();
//    int step2_connect_unlinked_points();
    std::string dump_figure(const REF_getter<figure> &f);
    REF_getter<figure> connect_2_figures(const REF_getter<figure>& f1, const REF_getter<figure>& f2);
    int get_nearest_point_in_figure(const REF_getter<figure> &f1, const point& p);
//    void link_neighbours(int p0);
    void step1_split_to_pairs();
    void link_neighbours2(const REF_getter<figure>&f);
    REF_getter<triangle> proceed_tiangle(int p0, int p2, int p3);
    bool validate_triangle(int a, int b, int c);
    int find_nearest(const point& p, const std::set<int> &ps);
    void flood();
    void proceed_on_angle_between_rebras(int p0, std::set<int> &unlinked_points, std::set<int> &active_points);
    real angle_between_3_points(int root, int a, int b);
    point cross_between_3_points(int root,int a, int b);





    std::map<std::set<int>,REF_getter<triangle> > all_triangles;




    void connect_unlinked_points();
//    void triangulate_figure(const REF_getter<figure> &F);
    void supress_figures();




    int figIdGen=0;

    std::map<int,REF_getter<figure>> all_figures;

    std::deque<REF_getter<rebro_container>> border_rebras_to_process;

    point rebro_center(const REF_getter<rebro_container> & rebro);
    int get_rebro_peer(const REF_getter<rebro_container> &rebro, int n);

    void load_points(const std::string& fn);
    void run(const std::string &fn, const std::string &fn_out);

    int find(const std::set<int> &searchSet1,const point &pt, const std::set<int> &rebro, const std::set<int> &except_pts, int refcount)
    {
        return algoFind__findBrutforce(searchSet1,pt,rebro,except_pts,refcount);
    }

    REF_getter<rebro_container> getRebroOrCreate(const std::set<int>& s, const char* comment);
    int process_point(int p1, const REF_getter<figure> &fig);
    void move_content(const REF_getter<figure>& to, const REF_getter<figure>&from);

    void calc_picture_size();
    bool line_len_ok(real len);

};

#endif // SURF_H
