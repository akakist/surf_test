#ifndef UTILS_H
#define UTILS_H
#include <stdlib.h>
#include <string>
#include "point.h"
#include "real.h"
#include <math.h>
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
std::vector<std::string> splitString(const char *seps, const std::string & src, size_t reserve);

#define D2Lmul 100000
inline real qw(real a)
{
    return a*a;
}
//inline long dist(const point& a, const point&b)
//{
//    return sqrt(qw(a.x-b.x)+qw(a.y-b.y)+qw(a.z-b.z)) * double(D2Lmul);
//}
inline real fdist(const point& a, const point&b)
{
    return sqrt(qw(a.x-b.x)+qw(a.y-b.y)+qw(a.z-b.z)) ;
}

std::string dump_set_int(const std::set<int> &s);


#endif // UTILS_H
