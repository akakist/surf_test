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
    void step1_split_to_rectangles();
    int step3_connect_figures();
//    void step4_process_points();
//    int step2_connect_unlinked_points();
    void stepN_create_figures_from_unlinked_points();
    std::string dump_figure(const REF_getter<figure> &f);
    bool figure_has_unbordered(const REF_getter<figure>& f);
    REF_getter<figure> connect_2_figures(const REF_getter<figure>& f1, const REF_getter<figure>& f2);
    int get_nearest_point_in_figure(const REF_getter<figure> &f1, const point& p);
    void make_rebring();
    void make_rebring2();
    void fill_rebring_1();



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
