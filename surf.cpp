#include "surf.h"
#include "utils.h"
#include <fstream>
#include <iostream>
#define MAX_N 3
void surface::run(const std::string &fn_in, const std::string& fn_out)
{
    load_points(fn_in);
    std::cout << "Calculate reperz" << std::endl;
    calculateReperz();
    std::cout << "done reperz" << std::endl;

    std::cout << "Calculate triangles" <<std::endl;

    /// seaching for 3 nearest points for each point
    /// Save standard deviation up to 3 points to R2_to_pts
    std::vector<std::set<int>> neighbours;
    std::map<double/*R2*/,int> R2_to_pts;
    neighbours.resize(pts.size());
    for(size_t i=0;i<pts.size();i++)
    {
        auto p3=find_3_NearestPointsByReperz(i);
        neighbours[i]=p3.second;
        R2_to_pts[p3.first]=i;
    }
    /// first connect the points with the smallest standard deviation while there are free slots

    std::vector/*pt*/< std::set<int/*ref pt*/> > drawedNeighbours4Pt;
    drawedNeighbours4Pt.resize(pts.size());
//    double AVG_R2=0;
//    int AVG_COUNT=0;
    for(auto& pt: R2_to_pts)
    {
        auto & n=neighbours[pt.second];
        for(auto &z: n)
        {
//            auto d=fdist(pts[pt.second], pts[z]);

//            if(AVG_COUNT>5 && sqrt(AVG_R2/(double)AVG_COUNT) * 100. < d)
//            {
//                /// If the distance between points is too greater than the average between points, then we do not insert these points.
////                continue;
//            }
//            AVG_R2=qw(d);

            /// If both points have free slots, then insert them mutually
            if(drawedNeighbours4Pt[pt.second].size()<MAX_N && drawedNeighbours4Pt[z].size()<MAX_N)
            {
                drawedNeighbours4Pt[pt.second].insert(z);
                drawedNeighbours4Pt[z].insert(pt.second);
//                AVG_COUNT++;
            }
        }
    }
    /// remove pts with 3 links from search container
    for(size_t i=0;i<drawedNeighbours4Pt.size();i++)
    {
        auto& p=drawedNeighbours4Pt[i];
        if(p.size()==MAX_N)
        {
            removePointFromDistPtsToRepers(i);
        }
    }
    /// scan all pts and add links if links < 2.
    for(size_t i=0;i<drawedNeighbours4Pt.size();i++)
    {
        auto& p=drawedNeighbours4Pt[i];
        while(p.size()<MAX_N-1)
        {
            auto neis=find_1_NearestByReperz(i, p);
            for(auto& nei:neis)
            {
                drawedNeighbours4Pt[i].insert(nei);
                drawedNeighbours4Pt[nei].insert(i);
                if(drawedNeighbours4Pt[i].size()==3)
                    removePointFromDistPtsToRepers(i);
                if(drawedNeighbours4Pt[nei].size()==3)
                    removePointFromDistPtsToRepers(nei);
            }

        }
    }



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

    std::set<std::set<int> > triangles;
    for(size_t i=0; i<drawedNeighbours4Pt.size(); i++)
    {
        auto sz=drawedNeighbours4Pt[i].size();
        switch(sz)
        {
        case 2:
        {
            std::set<int> tri;
            tri=drawedNeighbours4Pt[i];
            tri.insert(i);
            triangles.insert(std::move(tri));
        }
        break;
        case 1:
            throw std::runtime_error("wrong case 1:");
        case 0:
            throw std::runtime_error("wrong case 0:");
        case 3:
        {
            std::vector<int> t;
            t.reserve(3);
            for(auto& z: drawedNeighbours4Pt[i])
            {
                t.push_back(z);
            }
            triangles.insert({(int)i,t[0],t[1]});
            triangles.insert({(int)i,t[0],t[2]});
            triangles.insert({(int)i,t[1],t[2]});
        }
        break;
        default:
            throw std::runtime_error("wrong case default:"+ std::to_string(sz));
        }



    }
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
    for(size_t i=0; i<reperz.size(); i++)
    {
        auto &d_r_pt=distInRepersByPt[i];
        auto d=d_r_pt[pt];
        auto & dptr=distPtsToRepers[i];
        dptr[d].erase(pt);
        if(dptr[d].empty())
            dptr.erase(d);
    }
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

    point d=max-min;

    int step=1;
    for(int ix=-1; ix<2; ix+=step)
    {
        for(int iy=-1; iy<2; iy+=step)
        {
            for(int iz=-1; iz<2; iz+=step)
            {
                reperz.push_back(avg+point(d.x*(double)ix,d.y*(double)iy,d.z*(double)iz));
            }

        }
    }
    std::cout << "total reperz count " << reperz.size() << std::endl;

    distPtsToRepers.resize(reperz.size());
    distInRepersByPt.resize(reperz.size());
    for(auto& z: distInRepersByPt)
    {
        z.resize(pts.size());
    }
    for(size_t i=0; i<pts.size(); i++)
    {
        for(size_t j=0; j<reperz.size(); j++)
        {
            auto d=dist(pts[i],reperz[j]);
            distPtsToRepers[j][d].insert(i);
            distInRepersByPt[j][i]=d;
        }
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


std::pair<double,std::set<int> > surface::find_3_NearestPointsByReperz(int pt)
{
    //// calculate dist to each reper
    std::vector<long> distByReper;
    distByReper.reserve(pts.size());
    for(auto& z: reperz)
    {
        distByReper.push_back(dist(pts[pt],z));
    }
    std::set<int> resTmp;
    for(size_t i=0; i<distByReper.size(); i++)
    {
        /// find nearest upper_bound element in each reper
        /// exclude found neighbours
        std::vector<int> foundForReper;
        foundForReper.reserve(3);
        auto it=distPtsToRepers[i].upper_bound(distByReper[i]);
        while(foundForReper.size()<3 && it!=distPtsToRepers[i].end())
        {
            for(auto &z:it->second)
            {
                if(z!=pt)
                    foundForReper.push_back(z);
            }
//            if(foundForReper.size()>=3)
//                break;
            it++;

        }
        for(auto& z: foundForReper)
        {
            resTmp.insert(z);
        }

    }
    if(resTmp.empty())
        throw std::runtime_error("if(resTmp.empty())");

    /// use nearest element from set found from repers
    std::map<double,int> mp;
    for(auto& z: resTmp)
    {
        double d=fdist(pts[pt],pts[z]);
        mp.insert({d,z});

    }
    std::set<int> ret;
    if(mp.size()<3) throw std::runtime_error("if(mp.size()<3)");
    double R2=0;

    for(int i=0;i<3;i++)
    {
        R2+=qw(mp.begin()->first);
        ret.insert(mp.begin()->second);
        mp.erase(mp.begin());
    }

    return {sqrt(R2),ret};
}
std::set<int> surface::find_1_NearestByReperz(int pt, const std::set<int> &except_pts)
{
    //// calculate dist to each reper
    std::vector<long> distByReper;
    distByReper.reserve(pts.size());
    for(auto& z: reperz)
    {
        distByReper.push_back(dist(pts[pt],z));
    }
    std::set<int> resTmp;
    for(size_t i=0; i<distByReper.size(); i++)
    {
        /// find nearest upper_bound element in each reper
        /// exclude found neighbours
        std::vector<int> foundForReper;
        auto it=distPtsToRepers[i].upper_bound(distByReper[i]);
        while(foundForReper.empty() && it!=distPtsToRepers[i].end())
        {
            for(auto &z:it->second)
            {
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
//        throw std::runtime_error("if(resTmp.empty())");

    /// use nearest element from set found from repers
    int selected=*resTmp.begin();
    double min_d=std::numeric_limits<double>::max();
    for(auto& z: resTmp)
    {
        double d=dist(pts[pt],pts[z]);
        if(d<min_d)
        {
            min_d=d;
            selected=z;
        }

    }
    ret.insert(selected);
    return ret;

}
