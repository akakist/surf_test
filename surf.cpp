#include "surf.h"
#include "utils.h"
#include <fstream>
#include <iostream>
#include <deque>
#define MAX_N 3
std::string dump_rebro(const std::set<int>& rebro)
{
    std::string out;
    out+= std::to_string(*rebro.begin());
    out+= " ";
    out+= std::to_string(*rebro.rbegin());
    return out;

}
int get_rebro_peer(const std::set<int>& rebro,int n)
{
    if(!rebro.count(n))
        throw std::runtime_error("if(!rebro.count(n))");
    for(auto& z:rebro)
    {
        if(z!=n)
            return z;
    }
    throw std::runtime_error("not found");
}
void surface::run(const std::string &fn_in, const std::string& fn_out)
{
    load_points(fn_in);
    std::cout << "Calculate reperz" << std::endl;
    calculateReperz();
    std::cout << "done reperz" << std::endl;

    std::cout << "Calculate triangles" <<std::endl;

    drawedNeighbours4Pt.resize(pts.size());
    std::set<int> except;
    int p1=0;
    std::set<int> rebro;
    rebro.insert(p1);
    auto __p2=find_1_NearestByReperz2(pts[0],rebro,except,0);
    printf("__p2.size %d\n",__p2.size());
    std::deque<std::set<int>> rebraz;
    std::deque<std::set<int>> rebraz_copy;
    std::set<std::set<int> > triangles;
    if(__p2.size())
    {
        auto p2=*__p2.begin();
        std::set<int> rebro;
        rebro= {p1,p2};
        point rebro_center=(pts[*rebro.begin()]+pts[*rebro.rbegin()])/2;
        auto __p3=find_1_NearestByReperz2(rebro_center,rebro,except,0);
        if(__p3.size())
        {
            printf("_p3.size %d\n",__p3.size());
            auto p3=*__p3.begin();
            drawedNeighbours4Pt[p1].insert(p2);
            drawedNeighbours4Pt[p1].insert(p3);
            drawedNeighbours4Pt[p2].insert(p1);
            drawedNeighbours4Pt[p2].insert(p3);
            drawedNeighbours4Pt[p3].insert(p1);
            drawedNeighbours4Pt[p3].insert(p2);

            rebraz.push_back({p1,p3});
            rebraz.push_back({p2,p3});
            rebraz_copy.push_back({p1,p3});
            rebraz_copy.push_back({p2,p3});
            except.insert(p1);
            except.insert(p2);
            except.insert(p3);
            std::set<int> s= {p1,p2,p3};
            if(s.size()!=3)
                throw std::runtime_error("if(s.size()!=3)##2");
            triangles.insert(s);

        }
    }
    while(rebraz.size())
    {
        auto rebro=rebraz[0];
        rebraz.pop_front();
        auto p1=*rebro.begin();
        auto p2=*rebro.rbegin();
        point rebro_center=(pts[p1]+pts[p2])/2;
        auto _p3=find_1_NearestByReperz2(rebro_center,rebro,except,0);

        if(_p3.size())
        {

            auto p3=*_p3.begin();
            auto d=fdist(pts[p3],rebro_center);
            if(d>110)
            {
                continue;
            }

            rebraz.push_back({p1,p3});
            rebraz.push_back({p2,p3});

            rebraz_copy.push_back({p1,p3});
            rebraz_copy.push_back({p2,p3});

            drawedNeighbours4Pt[p1].insert(p2);
            drawedNeighbours4Pt[p1].insert(p3);
            drawedNeighbours4Pt[p2].insert(p1);
            drawedNeighbours4Pt[p2].insert(p3);
            drawedNeighbours4Pt[p3].insert(p1);
            drawedNeighbours4Pt[p3].insert(p2);
            std::set<int> s= {p1,p2,p3};
            if(s.size()!=3)
                throw std::runtime_error("if(s.size()!=3)##1");
            triangles.insert(s);

        }
        else continue;

    }

    std::map<int,std::set<int/*rebro_copy_idx*/> > ptIncludesRebraz;
    for(int i=0; i<rebraz_copy.size(); i++)
    {
        auto& r=rebraz_copy[i];
        ptIncludesRebraz[*r.begin()].insert(i);
        ptIncludesRebraz[*r.rbegin()].insert(i);
    }

    std::deque<std::set<int> > added_tri;
    /// connect points with 2 neigbours
    int matchcount=0;
//    reperFind=reperFind_copy;
    std::vector<std::set<int > > additional_links;
    additional_links.resize(pts.size());
    int extra_triangles=0;
    for(int i=0; i<drawedNeighbours4Pt.size(); i++)
    {
        if(drawedNeighbours4Pt[i].size()==2)
        {
            auto rebrazIdsForPt=ptIncludesRebraz[i];
            for(auto rId:rebrazIdsForPt)
            {
                auto rebro=rebraz_copy[rId];
                auto _p=find_1_NearestByReperz2((pts[*rebro.begin()]+pts[*rebro.rbegin()])/2.,rebro,{},2);
                if(_p.size())
                {
                    auto p1=*_p.begin();
                    auto p2=*rebro.begin();
                    auto p3=*rebro.rbegin();
                    triangles.insert({p1,p2,p3});
                    extra_triangles++;
                    additional_links[i]={p1,p2,p3};
                }

            }
        }
    }
    printf("extra_triangles %d\n",extra_triangles);
    for(int i=0;i<additional_links.size();i++)
    {
        for(auto& p:additional_links[i])
        {
            drawedNeighbours4Pt[i].insert(p);
            drawedNeighbours4Pt[p].insert(i);
        }
    }

    std::cout << "matchcount " << matchcount << std::endl;
    for(auto& z:added_tri)
    {
        auto s=z;
        std::vector<int>v;
        while(s.size())
        {
            v.push_back(*s.begin());
            s.erase(s.begin());
        }
        if(v.size()!=3)
            throw std::runtime_error("if(v.size()!=3)");
        drawedNeighbours4Pt[v[0]].insert(v[1]);
        drawedNeighbours4Pt[v[0]].insert(v[2]);
        drawedNeighbours4Pt[v[1]].insert(v[2]);
    }



    /// seaching for 3 nearest points for each point
    /// Save standard deviation up to 3 points to R2_to_pts
    ///
    ///
    ///

    std::map<int,int> cnt;
    for(auto & z: drawedNeighbours4Pt)
    {
        cnt[z.size()]++;
    }
    for(auto &z: cnt)
    {
        std::cout << "by count of linked neigbours " << z.first  << " " << z.second << std::endl;
    }

    std::cout << "Done triangles" <<std::endl;

    std::cout << "Print out" << std::endl;
    std::ofstream fout(fn_out);
    fout<<"*Nodes"<<std::endl;
    for(size_t i=0; i<pts.size(); i++)
    {
        fout<< i+1 <<", " << pts[i].x <<", " << pts[i].y << ", " << pts[i].z << std::endl;
    }
    fout<<"*Elements"<<std::endl;
    int idx=1;
    for(auto& t: triangles)
    {
        if(t.size()!=3)
            throw std::runtime_error("if(t.size()!=3)");
        fout<< idx ;
        for(auto&z: t)
        {
            fout<< ", " << z+1;
        }
        fout << std::endl;
        idx++;
    }
    std::cout << "Print done" << std::endl;

}

void surface::calculateReperz()
{
    point min=pts[0];
    point max=pts[0];

    point avg;
    for(size_t i=0; i<pts.size(); i++)
    {
        auto & p=pts[i];
        avg+=pts[i];
        if(p.x>max.x) max.x=p.x;
        if(p.y>max.y) max.y=p.y;
        if(p.z>max.z) max.z=p.z;

        if(p.x<min.x) min.x=p.x;
        if(p.y<min.y) min.y=p.y;
        if(p.z<min.z) min.z=p.z;

    }
    avg/=double(pts.size());
    figure_max_size=std::max(abs(max.x-min.x), std::max(abs(max.y-min.y),abs(max.z-min.z)));

    point d=max-min;
    d+=d+d;

    std::vector<double> idxs={-1,1};
    for(double ix: idxs)
    {
        for(double iy: idxs)
        {
            for(double iz:idxs)
            {
                reperz.push_back(avg+point(d.x*ix,d.y*iy,d.z*iz));
            }

        }
    }
    std::cout << "total reperz count " << reperz.size() << std::endl;

    reperFind2.distToPtsByReper.resize(reperz.size());
    for(auto& z:reperFind2.distToPtsByReper)
    {
        z.reserve(pts.size());
    }
    for(size_t r=0; r<reperz.size(); r++)
    {
        for(size_t p=0; p<pts.size(); p++)
        {
            auto d=fdist(pts[p],reperz[r]);
            reperFind2.distToPtsByReper[r].push_back({d,p});
        }
        std::sort(reperFind2.distToPtsByReper[r].begin(),reperFind2.distToPtsByReper[r].end());
        printf("rep size %d\n",reperFind2.distToPtsByReper[r].size());

    }


}
void surface::load_points(const std::string& fn)
{
    auto b=loadFile(fn);

    auto ls=splitString("\r\n",b, 70000);
    for(auto& l:ls)
    {
        if(l.size() && l[0]=='*')
            continue;
        auto v=splitString(" ",l,4);
        if(v.size()==4)
        {
            point p;
            int id=atoi(v[0].c_str());
            p.x=atof(v[1].c_str());
            p.y=atof(v[2].c_str());
            p.z=atof(v[3].c_str());
            pts.push_back(p);
        }
    }
    std::cout<< "Loaded pts " << pts.size() <<std::endl;

    return;
}

struct searchInReper
{
    int curIdx_up;
    int curIdx_down;
    double distFromReper;
    const point& reper;
    const std::vector<std::pair<double, int > > &seach_array;
    const std::set<int>& rebro;
    const std::set<int> &except_pts;
    const std::vector/*pt*/< std::set<int/*ref pt*/> > &drawedNeighbours4Pt;
    const int refcount;
    std::set<int> ptsInRange;
    searchInReper(const point& r,
                  std::vector<std::pair<double, int > >& sa,
                  const std::set<int>& _rebro,
                  const std::set<int>& _except,
                  const std::vector/*pt*/< std::set<int/*ref pt*/> > _drawedNeighbours4Pt,
                  int refc
                  ):reper(r),seach_array(sa),rebro(_rebro),except_pts(_except),drawedNeighbours4Pt(_drawedNeighbours4Pt),refcount(refc){}
    void init(const point& pt)
    {
        distFromReper=fdist(pt,reper);
        std::pair<double,int> s={distFromReper,0};
        auto it=std::upper_bound(seach_array.begin(),seach_array.end(),s);
        auto idx=std::distance(seach_array.begin(),it);
        printf("idx %d\n",idx);
        curIdx_up=idx;
        curIdx_down=idx;
    }
    bool canInsert(int p)
    {
        if(except_pts.count(p) || rebro.count(p)|| drawedNeighbours4Pt[p].size()!=refcount)
        {
//            printf("canInsert false\n");
            return false;
        }
//        printf("canInsert true\n");
        return true;

    }

    void expandRange(double d)
    {
        for(;curIdx_up<seach_array.size();curIdx_up++)
        {
            auto & sa=seach_array[curIdx_up];
            auto &p=sa.second;
            if(abs(sa.first-distFromReper)<d)
            {
                if(canInsert(sa.second))
                ptsInRange.insert(sa.second);
            }
            else break;
        }
        for(;curIdx_down>=0;curIdx_down--)
        {
            auto & sa=seach_array[curIdx_down];
            if(abs(sa.first-distFromReper)<d)
            {
                if(canInsert(sa.second))
                ptsInRange.insert(sa.second);
            }
            else break;
        }
//        printf("ptsInRange %d\n",ptsInRange.size());
    }

    std::set<int> intersection(const std::set<int> &ss)
    {
        std::set<int> ret;
        for(auto& s:ss)
        {
            if(ptsInRange.count(s))
                ret.insert(s);
        }
        return ret;
    }
};
std::set<int> surface::find_1_NearestByReperz2(const point &pt, const std::set<int>& rebro,const std::set<int> &except_pts, int refcount)
{
    //// calculate dist to each reper
    std::vector<double> distByReper;
    distByReper.resize(reperz.size());
    std::vector<searchInReper> sirs;
    sirs.reserve(reperz.size());
    for(int i=0;i<reperz.size();i++)
    {
        auto si=searchInReper(reperz[i],reperFind2.distToPtsByReper[i],rebro,except_pts,drawedNeighbours4Pt,refcount);
        si.init(pt);
        sirs.push_back(si);
    }
#define SPLICES 150.

    std::set<int> intersection;
    for(double d=figure_max_size/SPLICES;d<figure_max_size;d+=figure_max_size/SPLICES)
    {
        printf("D %lf\n",d);
        for(int s=0;s!=sirs.size();s++)
        {
            sirs[s].expandRange(d);
            printf("sirs[s].ptsInRange.size() %d (s %d)\n",sirs[s].ptsInRange.size(),s);
        }

        for(int s=0;s!=sirs.size();s++)
        {
            if(s==0)
                intersection=sirs[s].ptsInRange;
            else
                intersection=sirs[s].intersection(intersection);
            if(intersection.empty())
                break;

        }
        if(intersection.size())
            break;


    }
//                printf("final is size %d\n",intersection.size());

    std::set<int> ret;
    if(intersection.empty())
    {
        printf("if(intersection.empty())\n");
        return ret;
    }

    /// use nearest element from set found from repers
    int selected=*intersection.begin();
    double min_d=std::numeric_limits<double>::max();
    for(auto& z: intersection)
    {
        double d=fdist(pt,pts[z]);
        if(d<min_d)
        {
            min_d=d;
            selected=z;
        }

    }
    printf("selected %d\n",selected);
    ret.insert(selected);
    return ret;

}
