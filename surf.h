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
            throw std::runtime_error("if(opposize_pts.size()>2)");
        }
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
    std::set<REF_getter<rebro_container> > rebras;
    std::set<REF_getter<triangle>> neigbours;
    triangle(const std::set<int> & pt)
    {
        if(pt.size()!=3)
            throw std::runtime_error("if(pt.size()!=3)");
        points=pt;
    }
};

struct pointInfo
{
    pointInfo() {}
    std::map<std::set<int>,REF_getter<rebro_container>> rebras;

    std::set<REF_getter<triangle>> triangles;

    std::set<int> neighbours;
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
    std::vector<pointInfo> pointInfos;
    std::map<std::set<int>,REF_getter<rebro_container> > all_rebras;

    /// все закрашенные треугольники, которые мы выводим в obj mesh
    std::set<std::set<int> > triangles;
    real picture_size;
    std::map<std::set<int>,REF_getter<triangle> > all_triangles;

    /// не связанные (свободные точки)
    std::set<int> unlinked_points;

    /// точки на границе, которые могут быть соединены
    std::set<int> active_points;

    /// все связанные точки, будет использоваться для фильтрации unlinked_points
    /// при нахождении в unlinked_points ищем ближайшее в linked_points, если точка может соединяться, то ок,
    /// если не может, то выбрасываем найденную в unlinked_points точку
    std::set<int> linked_points;

    REF_getter<triangle> proceed_tiangle(int p0, int p2, int p3);
    int find_nearest(const point& p, const std::set<int> &ps);
    void flood();
    void proceed_add_new_point_between_rebras(int p0);
    int proceed_connection_between_tops(int p0);

    real angle_between_3_points(int root, int a, int b);
    point cross_between_3_points(int root,int a, int b);
    bool triangle_can_be_added(int p0, int p2, int pnearest, int p_opposite);
    int find_nearest_which_can_be_added(const point& pt, int p0, int p2, int p_opposite);

    void load_points(const std::string& fn);
    void run(const std::string &fn, const std::string &fn_out);

    REF_getter<rebro_container> getRebroOrCreate(const std::set<int>& s, const char* comment);

    void calc_picture_size();
    bool line_len_ok(real len);

};

#endif // SURF_H
