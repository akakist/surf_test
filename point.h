#ifndef POINT_H
#define POINT_H
#include <set>
struct rebro;
struct point
{
    double x,y,z;

    point():x(0),y(0),z(0) {}
    point(double x_, double y_, double z_):x(x_),y(y_),z(z_) {}
    point &div(double c)
    {
        x/=c;
        y/=c;
        z/=c;
        return *this;
    }
    point &add(const point &p) {
        x+=p.x;
        y+=p.y;
        z+=p.z;
        return *this;
    }
    point &sub(const point &p) {
        x-=p.x;
        y-=p.y;
        z-=p.z;
        return *this;
    }
//    friend  inline const point operator+(const point &, const point &);
//    friend  inline const point operator-(const point &, const point &);
//    friend  inline const point operator/(const point &, double);

};
//inline const point operator+(const point &p1, const point &p2)
//{
//    return point(p1.x+p2.x, p1.y+p2.y,p1.z+p2.z);
//}
//inline const point operator-(const point &p1, const point &p2)
//{
//    return point(p1.x-p2.x, p1.y-p2.y,p1.z-p2.z);
//}
//inline const point operator/(const point &p,double c)
//{
//    return point(p.x/c, p.y/c, p.z/c);
//}

#endif // POINT_H
