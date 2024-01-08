#ifndef POINT_H
#define POINT_H
#include <set>
#include "real.h"
#include <cmath>
struct rebro;
struct point
{
    real x,y,z;

    point():x(0),y(0),z(0) {}
    point(real x_, real y_, real z_):x(x_),y(y_),z(z_) {}
    point &div(real c)
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
    point &operator/=(double c)
    {
        x/=c;
        y/=c;
        z/=c;
        return *this;
    }
    point &operator+=(const point &p) {
        x+=p.x;
        y+=p.y;
        z+=p.z;
        return *this;
    }
    point &operator-=(const point &p) {
        x-=p.x;
        y-=p.y;
        z-=p.z;
        return *this;
    }
    friend  inline const point operator+(const point &, const point &);
    friend  inline const point operator-(const point &, const point &);
    friend  inline const point operator/(const point &, double);

};
inline const point operator+(const point &p1, const point &p2)
{
    return point(p1.x+p2.x, p1.y+p2.y,p1.z+p2.z);
}
inline const point operator-(const point &p1, const point &p2)
{
    return point(p1.x-p2.x, p1.y-p2.y,p1.z-p2.z);
}
inline const point operator/(const point &p,double c)
{
    return point(p.x/c, p.y/c, p.z/c);
}
namespace Angle
{
    inline real dot(const point& a, const point& b)  //calculates dot product of a and b
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    inline real mag(const point& a)  //calculates magnitude of a
    {
        return std::sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
    }
    inline real angle(const point& v1,const point& v2) // grads
    {
        return std::acos(dot(v1,v2)/(mag(v1)*mag(v2)))*(180/M_PI);
    }
}

#endif // POINT_H
