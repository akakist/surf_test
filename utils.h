#ifndef UTILS_H
#define UTILS_H
#include <stdlib.h>
#include <string>
#include "point.h"
struct st_malloc
{
private:
    st_malloc(const st_malloc&);             // Not defined
    st_malloc& operator=(const st_malloc&);  // Not defined
public:
    unsigned char *buf;
    st_malloc(size_t size)
    {
        buf = (unsigned char*)malloc(size + 1);
    }
    ~st_malloc()
    {
        if (buf)
        {
            free(buf);
            buf = NULL;
        }
    }

};
std::string loadFile(const std::string& name);
std::vector<std::string> splitString(const char *seps, const std::string & src);

#define D2Lmul 100000

inline double qw(double a)
{
    return a*a;
}
inline long dist(const point& a, const point&b)
{
    return sqrt(qw(a.x-b.x)+qw(a.y-b.y)+qw(a.z-b.z)) * double(D2Lmul);
}
inline double fdist(const point& a, const point&b)
{
    return sqrt(qw(a.x-b.x)+qw(a.y-b.y)+qw(a.z-b.z)) * double(D2Lmul);
}


#endif // UTILS_H
