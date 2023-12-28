#ifndef SURF_H
#define SURF_H
#include "point.h"
#include <vector>
#include <set>
#include <map>
#define SLICES 100

struct surface
{
    surface()
    {
    }
    std::vector<point> pts;

    struct _boxes
    {
        point min;
        point max;
        point figure_size;
        point figure_size_delta;
        std::vector<int> calcBox(const point &p)
        {
            auto pnorm=p-min;
            auto yashik=pnorm/figure_size_delta;
            std::vector<int> yashik_i={(int)yashik.x,(int)yashik.y,(int)yashik.z};
            return yashik_i;

        }

        std::map<int,std::vector<int> > pt2box;
        std::vector<std::vector<std::vector<std::set<int>>>> pts_in_box;
        std::set<int> getContentForPt(int pt, int depth)
        {
            return getContent2(pt2box[pt],depth);
        }
        std::set<int> getContent(const std::vector<int>& center, int depth)
        {
            std::set<int> ret;
            for(int x=center[0]-depth;x<center[0]+depth;x++)
            {
                for(int y=center[1]-depth;y<center[1]+depth;y++)
                {
                    for(int z=center[2]-depth;z<center[2]+depth;z++)
                    {
                        auto& s=pts_in_box[x][y][z];
                        for(auto &a:s)
                        ret.insert(a);
                    }

                }

            }
            return ret;
        }
        std::set<int> getContent2(const std::vector<int>& center, int depth)
        {
            std::set<int> ret;
            for(int a:{center[0]-depth,center[0]+depth-1})
            {
                for(int b=center[1]-depth;b<center[1]+depth;b++)
                {
                    for(int c=center[2]-depth;c<center[2]+depth;c++)
                    {

                        auto& s=pts_in_box[a][b][c];
                        for(auto &a:s)
                        ret.insert(a);
                    }

                }

            }
            for(int a:{center[1]-depth,center[1]+depth-1})
            {
                for(int b=center[0]-depth;b<center[0]+depth;b++)
                {
                    for(int c=center[2]-depth;c<center[2]+depth;c++)
                    {

                        auto& s=pts_in_box[b][a][c];
                        for(auto &a:s)
                        ret.insert(a);
                    }

                }

            }
            for(int a:{center[2]-depth,center[2]+depth-1})
            {
                for(int b=center[0]-depth;b<center[0]+depth;b++)
                {
                    for(int c=center[1]-depth;c<center[1]+depth;c++)
                    {

                        auto& s=pts_in_box[b][c][a];
                        for(auto &a:s)
                        ret.insert(a);
                    }

                }

            }

            return ret;
        }
    };
    _boxes boxes;

    std::vector/*pt*/< std::set<int/*ref pt*/> > drawedNeighbours4Pt;

    std::set<int> find_1_NearestByReperz(const point &pt, const std::set<int> &rebro, const std::set<int> &except_pts, int refcount);
    std::set<int> find_1_NearestByReperz2(const point &pt, const std::set<int>& rebro,const std::set<int> &except_pts, int refcount);

    void removePointFromDistPtsToRepers(int pt);
    void load_points(const std::string& fn);
    void calculateReperz();
    void run(const std::string &fn, const std::string &fn_out);

};

#endif // SURF_H
