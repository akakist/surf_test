#ifndef SURF_H
#define SURF_H
#include <vector>
#include <set>
#include <map>
#include <deque>
#include <string.h>
#include <math.h>


std::string loadFile(const std::string& name);
std::vector<std::string> splitString(const char *seps, const std::string & src, size_t reserve);

#define D2Lmul 100000
inline double qw(double a)
{
    return a*a;
}



std::string dump_set_int(const std::set<int> &s);

struct Point
{
    double x,y,z;

    Point():x(0),y(0),z(0) {}
    Point(double x_, double y_, double z_):x(x_),y(y_),z(z_) {}
    Point &div(double c)
    {
        x/=c;
        y/=c;
        z/=c;
        return *this;
    }
    Point &add(const Point &p) {
        x+=p.x;
        y+=p.y;
        z+=p.z;
        return *this;
    }
    Point &sub(const Point &p) {
        x-=p.x;
        y-=p.y;
        z-=p.z;
        return *this;
    }
    Point &operator/=(double c)
    {
        x/=c;
        y/=c;
        z/=c;
        return *this;
    }
    Point &operator+=(const Point &p) {
        x+=p.x;
        y+=p.y;
        z+=p.z;
        return *this;
    }
    Point &operator-=(const Point &p) {
        x-=p.x;
        y-=p.y;
        z-=p.z;
        return *this;
    }
    friend  inline const Point operator+(const Point &, const Point &);
    friend  inline const Point operator-(const Point &, const Point &);
    friend  inline const Point operator/(const Point &, double);

};
inline const Point operator+(const Point &p1, const Point &p2)
{
    return Point(p1.x+p2.x, p1.y+p2.y,p1.z+p2.z);
}
inline const Point operator-(const Point &p1, const Point &p2)
{
    return Point(p1.x-p2.x, p1.y-p2.y,p1.z-p2.z);
}
inline const Point operator/(const Point &p,double c)
{
    return Point(p.x/c, p.y/c, p.z/c);
}
inline double fdist(const Point& a, const Point&b)
{
    return sqrt(qw(a.x-b.x)+qw(a.y-b.y)+qw(a.z-b.z)) ;
}
namespace Angle
{
    inline double dot(const Point& a, const Point& b)  //calculates dot product of a and b
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    inline double mag(const Point& a)  //calculates magnitude of a
    {
        return sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
    }
    inline double angle(const Point& v1,const Point& v2) // grads
    {
        return acos(dot(v1,v2)/(mag(v1)*mag(v2)))*(180/M_PI);
    }
}
struct Edge
{
    /// 2 точки, формирующие ребро
    std::set<int> points;
    /// дополнительная точка, формирующая треугольник.
    /// Их всего может быть 2, если одна, то к ребру можно еще присоединять треугольник
    std::set<int> opposize_pts;
    const char *comment=nullptr;

    void add_opposite_pts(int p)
    {
        opposize_pts.insert(p);
        if(opposize_pts.size()>2)
        {
            throw std::runtime_error("if(opposize_pts.size()>2)");
        }
    }
    Edge() {}
    Edge(const std::set<int>&s, const char* comm):points(s), comment(comm) {
        if(s.size()!=2)
            throw std::runtime_error("if(s.size()!=2)");

    }

};
struct triangle
{
    int id;
    std::set<int> points;
    std::set<std::shared_ptr<Edge> > edges;
    std::set<std::shared_ptr<triangle>> neigbours;
    Point center;
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
    std::map<std::set<int>,std::shared_ptr<Edge>> edges;

    std::set<std::shared_ptr<triangle>> triangles;

    std::set<int> neighbours;
    std::set<std::shared_ptr<Edge> > not_filles_rebras()
    {
        std::set<std::shared_ptr<Edge> > ret;
        for(auto& r:edges)
        {
            if(r.second->opposize_pts.size()<2)
            {
                ret.insert(r.second);
            }
        }
        return ret;
    }
    void add_to_edges(const std::shared_ptr<Edge>& r)
    {
        edges.insert({r->points,r});
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
    ~surface();

    std::vector<Point > pts;
    std::vector<pointInfo> pointInfos;
    std::map<std::set<int>,std::shared_ptr<Edge> > all_edges;

    /// все закрашенные треугольники, которые мы выводим в obj mesh
    std::set<std::set<int> > triangles;

//    std::map<int,std::shared_ptr<triangle>> all_triangles;
    double picture_size;

    /// не связанные (свободные точки)
    std::set<int> unlinked_points;

    /// точки на границе, которые могут быть соединены
    std::set<int> active_points;

    /// все связанные точки, будет использоваться для фильтрации unlinked_points
    /// при нахождении в unlinked_points ищем ближайшее в linked_points, если точка может соединяться, то ок,
    /// если не может, то выбрасываем найденную в unlinked_points точку
    std::set<int> linked_points;
//    std::deque<Point> surface_pts;

    /// средняя дистанция между точками
    double avg_dist;


    std::shared_ptr<triangle> make_tiangle(int p0, int p2, int p3);
    int find_nearest(const Point& p, const std::set<int> &ps);

    void flood();
    void proceed_add_new_point_between_edges(int p0, std::deque<int> &interested);
    int proceed_connection_between_tops(int p0);

    double angle_between_3_points(int root, int a, int b);
    Point cross_between_3_points(int root,int a, int b);
    bool triangle_can_be_added(int p0, int p2, int pnearest, int p_opposite, double distance, std::set<int> &erased_pts);
    int find_nearest_which_can_be_added(const Point& pt, int p0, int p2, int p_opposite);

    void load_points(const std::string& fn);
    void run(const std::string &fn, const std::string &fn_out);

    std::shared_ptr<Edge> get_edge_or_create(const std::set<int>& s, const char* comment);

    void calc_picture_size();
    bool line_len_ok(double len);

};

#endif // SURF_H
