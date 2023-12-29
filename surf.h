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
    private:
        std::vector<std::vector<std::vector<std::set<int>>>> pts_in_box;
    public:
        void resize_pts_from_box()
        {
            pts_in_box.resize(SLICES+1);
            for(int i=0;i<SLICES;i++)
            {
                pts_in_box[i].resize(SLICES+1);
                for(int j=0;j<SLICES;j++)
                {
                    pts_in_box[i][j].resize(SLICES+1);

                }
            }

        }
        bool verify_box(const std::vector<int>& b)
        {
            if(b.size()!=3)
                return false;
            if(b[0]>0 && b[0]<SLICES
                    &&b[1]>0 && b[1]<SLICES
                    &&b[2]>0 && b[2]<SLICES
                    )
                return true;
//            printf("wrong box %d %d %d\n",b[0],b[1],b[2]);
            return false;
        }
        std::set<int> getContentForPt(int pt, int depth)
        {
            return getContent2(pt2box[pt],depth);
        }

        std::set<int>& get_pts_from_box(const std::vector<int>& c)
        {
            static std::set<int>empty={};
            if(verify_box(c))
            {
                return pts_in_box[c[0]][c[1]][c[2]];
            }
            return empty;

        }
        std::set<int> getContent2(const std::vector<int>& center, int depth)
        {
            std::set<int> ret;
            if(depth==0)
                return get_pts_from_box(center);

            for(int a:{center[0]-depth,center[0]+depth})
            {
                for(int b=center[1]-depth;b<=center[1]+depth;b++)
                {
                    for(int c=center[2]-depth;c<=center[2]+depth;c++)
                    {
                        if(!verify_box({a,b,c}))
                            continue;
                        auto& s=get_pts_from_box({a,b,c});
                        for(auto &a:s)
                        ret.insert(a);
                    }

                }

            }
            for(int a:{center[1]-depth,center[1]+depth})
            {
                for(int b=center[0]-depth;b<=center[0]+depth;b++)
                {
                    for(int c=center[2]-depth;c<=center[2]+depth;c++)
                    {

                        auto& s=get_pts_from_box({b,a,c});
                        for(auto &a:s)
                        ret.insert(a);
                    }

                }

            }
            for(int a:{center[2]-depth,center[2]+depth})
            {
                for(int b=center[0]-depth;b<=center[0]+depth;b++)
                {
                    for(int c=center[1]-depth;c<=center[1]+depth;c++)
                    {

                        auto& s=get_pts_from_box({b,c,a});
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
