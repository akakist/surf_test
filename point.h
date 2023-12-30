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

};
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
    inline real angle(const point& v1,const point& v2)
    {
        return std::acos(dot(v1,v2)/(mag(v1)*mag(v2)));
    }
}

/*
#include <cmath>



int main()
{
    Vec3 v1, v2;

    v1.x = 203;
    v1.y = 355;
    v1.z = 922;

    v2.x = 6;
    v2.y = 13;
    v2.z = 198;

    float angle = std::acos(dot(v1,v2)/(mag(v1)*mag(v2)));
}
*/
#endif // POINT_H
