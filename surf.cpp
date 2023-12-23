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
    neighboursForPt.resize(pts.size());
    std::cout << "Calculate reperz" << std::endl;
    calculateReperz();
    std::cout << "done reperz" << std::endl;

    std::cout << "Calculate triangles" <<std::endl;
    while(ptsToSearch.size())
    {

        int p0=*ptsToSearch.begin();
        int p1=findNearestByReperz(p0,neighboursForPt[p0]);
        if(p1==p0)
            throw std::runtime_error("if(p1==p0)");
        neighboursForPt[p0].insert(p1);
        neighboursForPt[p1].insert(p0);
        auto ns0=neighboursForPt[p0].size();
        auto ns1=neighboursForPt[p1].size();
        if(ns0>=3)
        {
            removePointFromDistPtsToRepers(p0);
            ptsToSearch.erase(p0);
        }
        if(ns1>=3)
        {
            removePointFromDistPtsToRepers(p1);
            ptsToSearch.erase(p1);
        }
        if(ns0==2)
        {
            ptsToSearch.erase(p0);
        }
        if(ns1==2)
        {
            ptsToSearch.erase(p1);

        }
    }
    std::cout << "Done triangles" <<std::endl;

    std::set<std::set<int> > triangles;
    for(size_t i=0; i<neighboursForPt.size(); i++)
    {
        switch(neighboursForPt[i].size())
        {
        case 2:
        {
            std::set<int> tri;
            tri=neighboursForPt[i];
            tri.insert(i);
            triangles.insert(std::move(tri));
        }
        break;
        case 1:
            throw std::runtime_error("case 1:");
        case 3:
        {
            std::vector<int> t;
            for(auto& z: neighboursForPt[i])
            {
                t.push_back(z);
            }
            triangles.insert({(int)i,t[0],t[1]});
            triangles.insert({(int)i,t[0],t[2]});
            triangles.insert({(int)i,t[1],t[2]});
        }
        break;
        default:
            throw std::runtime_error("default:");
        }



    }
    std::cout << "Print out" << std::endl;
    std::ofstream fout(fn_out);
    fout<<"*Nodes"<<std::endl;
    for(size_t i=0; i<pts.size(); i++)
    {
        fout<< ptIdx2TaskId[i] <<", " << pts[i].x <<", " << pts[i].y << ", " << pts[i].z << std::endl;
    }
    fout<<"*Elements"<<std::endl;
    int idx=1;
    for(auto& t: triangles)
    {
        fout<< idx ;
        for(auto&z: t)
        {
            fout<< ", " << ptIdx2TaskId[z];
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
        auto d=distInRepersByPt[i][pt];
        distPtsToRepers[i][d].erase(pt);
        if(distPtsToRepers[i][d].empty())
            distPtsToRepers[i].erase(d);
    }
}


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


int surface::findNearestByReperz(int pt, const std::set<int> &except_pts)
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
    if(resTmp.empty())
        throw std::runtime_error("if(resTmp.empty())");

    /// use nearest element from set found from repers
    int selected=*resTmp.begin();
    double min_d=10e50;
    for(auto& z: resTmp)
    {
        double d=dist(pts[pt],pts[z]);
        if(d<min_d)
        {
            min_d=d;
            selected=z;
        }

    }
    return selected;

}
