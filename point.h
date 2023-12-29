#ifndef POINT_H
#define POINT_H
#include <set>
#include "real.h"
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

#endif // POINT_H
