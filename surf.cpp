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
    auto __p2=find_1_NearestByReperz(pts[0],rebro,except,0);
    std::deque<std::set<int>> rebraz;
    std::deque<std::set<int>> rebraz_copy;
    std::set<std::set<int> > triangles;
    if(__p2.size())
    {
        auto p2=*__p2.begin();
        std::set<int> rebro;
        rebro= {p1,p2};
        point rebro_center=(pts[*rebro.begin()]+pts[*rebro.rbegin()])/2;
        auto __p3=find_1_NearestByReperz(rebro_center,rebro,except,0);
        if(__p3.size())
        {
            auto p3=*__p3.begin();
            drawedNeighbours4Pt[p1].insert(p2);
            drawedNeighbours4Pt[p1].insert(p3);
            drawedNeighbours4Pt[p2].insert(p1);
            drawedNeighbours4Pt[p2].insert(p3);
            drawedNeighbours4Pt[p3].insert(p1);
            drawedNeighbours4Pt[p3].insert(p2);
            removePointFromDistPtsToRepers(p1);
            removePointFromDistPtsToRepers(p2);
            removePointFromDistPtsToRepers(p3);

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
        auto _p3=find_1_NearestByReperz(rebro_center,rebro,except,0);

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
            removePointFromDistPtsToRepers(p1);
            removePointFromDistPtsToRepers(p2);
            removePointFromDistPtsToRepers(p3);
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
    for(int i=0; i<drawedNeighbours4Pt.size(); i++)
    {
        if(drawedNeighbours4Pt[i].size()==2)
        {
            auto& rsIdx=ptIncludesRebraz[i];

            for(auto& rIdx:rsIdx)
            {
                auto rebro=rebraz_copy[rIdx];
                int peer=get_rebro_peer(rebro,i);
                auto& rs2idx=ptIncludesRebraz[peer];
                bool matched=false;
                for(auto& r2idx:rs2idx)
                {
                    auto rebro2=rebraz_copy[r2idx];
                    int peer2=get_rebro_peer(rebro2,peer);
                    if(peer2==i)
                    {
                        matched=true;
                        matchcount++;
                    }
                }
                if(matched)
                {
                    for(auto& r2idx:rs2idx)
                    {
                        auto rebro2=rebraz_copy[r2idx];
                        int peer2=get_rebro_peer(rebro2,peer);
                        if(peer2==i)
                            continue;
                        if(drawedNeighbours4Pt[peer2].size()==2)
                        {
                            std::set<int> s= {i,peer,peer2};
                            if(s.size()!=3)
                                throw std::runtime_error("if(s.size()!=3)##1");
                            triangles.insert(s);
                            added_tri.push_back(s);

                        }
                    }

                }

            }
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
void surface::removePointFromDistPtsToRepers(int pt)
{
    auto c=boxes.pt2box[pt];
    boxes.pts_in_box[c[0]][c[1]][c[2]].erase(pt);
    boxes.pt2box.erase(pt);



}

void surface::calculateReperz()
{
    boxes.min=pts[0];
    boxes.max=pts[0];

    point avg;
    for(size_t i=0; i<pts.size(); i++)
    {
        auto & p=pts[i];
        avg+=pts[i];
        if(p.x>boxes.max.x) boxes.max.x=p.x;
        if(p.y>boxes.max.y) boxes.max.y=p.y;
        if(p.z>boxes.max.z) boxes.max.z=p.z;

        if(p.x<boxes.min.x) boxes.min.x=p.x;
        if(p.y<boxes.min.y) boxes.min.y=p.y;
        if(p.z<boxes.min.z) boxes.min.z=p.z;

    }
    avg/=double(pts.size());


    boxes.figure_size=boxes.max-boxes.min;

    boxes.figure_size_delta=boxes.figure_size/(SLICES+1);

    boxes.pts_in_box.resize(SLICES+1);
    for(int i=0;i<SLICES;i++)
    {
        boxes.pts_in_box[i].resize(SLICES+1);
        for(int j=0;j<SLICES;j++)
        {
            boxes.pts_in_box[i][j].resize(SLICES+1);

        }
    }
    for(size_t i=0; i<pts.size(); i++)
    {
        auto&p=pts[i];
        auto ya=boxes.calcBox(p);

        printf("yashik %d %d %d\n",ya[0],ya[1],ya[2]);
        boxes.pts_in_box[ya[0]][ya[1]][ya[2]].insert(i);
        boxes.pt2box[i]=ya;
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
#ifdef KALL
std::set<int> surface::find_1_NearestByReperz2(const point &pt, const std::set<int>& rebro,const std::set<int> &except_pts, int refcount)
{
    //// calculate dist to each reper
    std::vector<long> distByReper;
    distByReper.reserve(pts.size());
    for(auto& z: reperz)
    {
        distByReper.push_back(dist(pt,z));
    }
    std::set<int> resTmp;
    for(size_t i=0; i<distByReper.size(); i++)
    {
#define nPTS 30
        /// find nearest upper_bound element in each reper
        /// exclude found neighbours
        std::vector<int> foundForReper;
        foundForReper.reserve(nPTS);
        auto it=reperFind.distPtsToRepers[i].upper_bound(distByReper[i]);
        while(foundForReper.size()<nPTS && it!=reperFind.distPtsToRepers[i].end())
        {
            for(auto &z:it->second)
            {
                if(drawedNeighbours4Pt[z].size()!=refcount)
                    continue;
                if(rebro.count(z))
                    continue;
                if(!except_pts.count(z))
                {
                    foundForReper.push_back(z);
                }
            }
            it++;
        }
        for(auto& z: foundForReper)
        {
            resTmp.insert(z);
        }

    }
    std::set<int> ret;
    if(resTmp.empty())
        return ret;

    /// use nearest element from set found from repers
    int selected=*resTmp.begin();
    double min_d=std::numeric_limits<double>::max();
    for(auto& z: resTmp)
    {
        double d=dist(pt,pts[z]);
        if(d<min_d)
        {
            min_d=d;
            selected=z;
        }

    }
    ret.insert(selected);
    return ret;

}
#endif
std::set<int> surface::find_1_NearestByReperz(const point &pt, const std::set<int>& rebro,const std::set<int> &except_pts, int refcount)
{
    std::set<int> s;

    for(int depth=0;s.size()<20 && depth<20;depth++)
    {
        auto b=boxes.getContent2(boxes.calcBox(pt),depth);
        for(auto& z: b)
        {
            if(drawedNeighbours4Pt[z].size()!=refcount)
                continue;
            if(rebro.count(z))
                continue;
            if(!except_pts.count(z))
            {
                s.insert(z);
            }

        }
    }
    int selected=*s.begin();
    double min_d=std::numeric_limits<double>::max();
    for(auto& z: s)
    {
        double d=dist(pt,pts[z]);
        if(d<min_d)
        {
            min_d=d;
            selected=z;
        }

    }
    std::set<int> ret;
    ret.insert(selected);
    return ret;

#ifdef KALL
    //// calculate dist to each reper
    std::vector<long> distByReper;
    distByReper.reserve(pts.size());
    for(auto& z: reperz)
    {
        distByReper.push_back(dist(pt,z));
    }
    std::set<int> resTmp;
    for(size_t i=0; i<distByReper.size(); i++)
    {
#define nPTS 30
        /// find nearest upper_bound element in each reper
        /// exclude found neighbours
        std::vector<int> foundForReper;
        foundForReper.reserve(nPTS);
        auto it=reperFind.distPtsToRepers[i].upper_bound(distByReper[i]);
        while(foundForReper.size()<nPTS && it!=reperFind.distPtsToRepers[i].end())
        {
            for(auto &z:it->second)
            {
                if(drawedNeighbours4Pt[z].size()!=refcount)
                    continue;
                if(rebro.count(z))
                    continue;
                if(!except_pts.count(z))
                {
                    foundForReper.push_back(z);
                }
            }
            it++;
        }
        for(auto& z: foundForReper)
        {
            resTmp.insert(z);
        }

    }
    std::set<int> ret;
    if(resTmp.empty())
        return ret;

    /// use nearest element from set found from repers
    int selected=*resTmp.begin();
    double min_d=std::numeric_limits<double>::max();
    for(auto& z: resTmp)
    {
        double d=dist(pt,pts[z]);
        if(d<min_d)
        {
            min_d=d;
            selected=z;
        }

    }
    ret.insert(selected);
    return ret;
#endif
}
