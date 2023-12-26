#include "surf.h"
#include "utils.h"
#include <fstream>
#include <iostream>
void surface::run(const std::string &fn_in, const std::string& fn_out)
{
    load_points(fn_in);
    for(size_t i=0; i<pts.size(); i++)
    {
        ptsToSearch.insert(i);
    }
//    neighboursForPt.resize(pts.size());
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
        auto p3=find3NearestByReperz(i);
        neighbours[i]=p3.second;
        R2_to_pts[p3.first]=i;
    }
    /// first connect the points with the smallest standard deviation while there are free slots

    std::vector/*pt*/< std::set<int/*ref pt*/> > drawedNeighbours4Pt;
    drawedNeighbours4Pt.resize(pts.size());
    double AVG_R2=0;
    int AVG_COUNT=0;
    for(auto& pt: R2_to_pts)
    {
        auto & n=neighbours[pt.second];
        for(auto &z: n)
        {
            auto d=fdist(pts[pt.second], pts[z]);

            if(AVG_COUNT>5 && sqrt(AVG_R2/(double)AVG_COUNT) * 100. < d)
            {
                /// If the distance between points is too greater than the average between points, then we do not insert these points.
//                continue;
            }
            AVG_R2=qw(d);

            /// If both points have free slots, then insert them mutually
            if(drawedNeighbours4Pt[pt.second].size()<3 && drawedNeighbours4Pt[z].size()<3)
            {
                drawedNeighbours4Pt[pt.second].insert(z);
                drawedNeighbours4Pt[z].insert(pt.second);
                AVG_COUNT++;
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
        std::cout << "by count " << z.first  << " " << z.second << std::endl;
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
            continue;
            throw std::runtime_error("case 1:");
        case 0:
            continue;
            throw std::runtime_error("case 0:");
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
            throw std::runtime_error("default:"+ std::to_string(sz));
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
#ifdef KALL
void surface::removePointFromDistPtsToRepers(int pt)
{
    for(size_t i=0; i<reperz.size(); i++)
    {
        auto d=distInRepersByPt[i][pt];
        distPtsToRepers[i][d].erase(pt);
        if(distPtsToRepers[i][d].empty())
            distPtsToRepers[i].erase(d);
    }
}
#endif


void surface::calculateReperz()
{
    point min=pts[0];
    point max=pts[0];

    point avg;
    for(size_t i=0; i<pts.size(); i++)
    {
        avg+=pts[i];
        if(pts[i].x>max.x) max.x=pts[i].x;
        if(pts[i].y>max.y) max.y=pts[i].y;
        if(pts[i].z>max.z) max.z=pts[i].z;

        if(pts[i].x<min.x) min.x=pts[i].x;
        if(pts[i].y<min.y) min.y=pts[i].y;
        if(pts[i].z<min.z) min.z=pts[i].z;


    }
    avg/=double(pts.size());

    point d=max-min;

    for(int ix=-1; ix<2; ix++)
    {
        for(int iy=-1; iy<2; iy++)
        {
            for(int iz=-1; iz<2; iz++)
            {
                reperz.push_back(avg+point(d.x*(double)ix,d.y*(double)iy,d.z*(double)iz));
            }

        }
    }

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
    auto ls=splitString("\r\n",b);
    for(auto& l:ls)
    {
        if(l.size() && l[0]=='*')
            continue;
        auto v=splitString(" ",l);
        if(v.size()==4)
        {
            point p;
            int id=atoi(v[0].c_str());
            p.x=atof(v[1].c_str());
            p.y=atof(v[2].c_str());
            p.z=atof(v[3].c_str());
            ptIdx2TaskId.push_back(id);
            pts.push_back(p);
        }
    }
    std::cout<< "Loaded pts " << pts.size() <<std::endl;

    return;
}


std::pair<double,std::set<int> > surface::find3NearestByReperz(int pt)
{
    //// calculate dist to each reper
    std::vector<long> distByReper;
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
        while(foundForReper.empty() && it!=distPtsToRepers[i].end())
        {
            for(auto &z:it->second)
            {
                if(z!=pt)
                    foundForReper.push_back(z);
            }
            if(foundForReper.size()>=3)
                break;
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
//    std::vector<int> selected=*resTmp.begin();
    std::map<double,int> mp;
//    double min_d=10e50;
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

//    return selected;

}
