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
point surface::rebro_center(const REF_getter<rebro_container> & rebro)
{
    point p;
    for(auto& s: rebro->points)
    {
        p.add(pts[s]);
    }
    p.div(rebro->points.size());
    return p;
}
void surface::process_rebras(int refcount, bool append_new_rebras)
{
    while(rebras_to_process.size())
    {
        if(rebras_to_process.size()%100==0)
            printf("rebras remained %d\n",rebras_to_process.size());
        auto rebro12=rebras_to_process[0];
        rebras_to_process.pop_front();
        if(rebro12->opposize_pts.size()>2)
            continue;

        int p1=*rebro12->points.begin();
        int p2=*rebro12->points.rbegin();

        point rc=rebro_center(rebro12);

        auto p3=find(rc,rebro12->points,rebro12->opposize_pts,refcount);
//        printf("p3 %d\n",p3);


        {
            auto d=fdist(pts[p3],rc);
            if(d>101)
            {
                continue;
            }
            std::set<int> s_23({p2,p3});
            std::set<int> s_13({p1,p3});
            REF_getter<rebro_container> rebro23(NULL);
            REF_getter<rebro_container> rebro13(NULL);
            {
                auto it1=all_rebras.find(s_23);
                if(it1!=all_rebras.end())
                {
                    rebro23=it1->second;
                }
                else {
                    rebro23= new rebro_container(s_23);

                    all_rebras.insert({s_23,rebro23});
                    pointInfos[p2].rebras.insert({rebro23->points,rebro23});
                    pointInfos[p3].rebras.insert({rebro23->points,rebro23});

                }
            }

            {
                auto it2=all_rebras.find(s_13);
                if(it2!=all_rebras.end())
                {
                    rebro13=it2->second;

                }
                else {
                    rebro13= new rebro_container(s_13);
                    all_rebras.insert({s_13,rebro13});
                    pointInfos[p1].rebras.insert({rebro13->points,rebro13});
                    pointInfos[p3].rebras.insert({rebro13->points,rebro13});
                }
            }
            if(append_new_rebras)
            {
            rebras_to_process.push_back(rebro23);
            rebras_to_process.push_back(rebro13);
            }

            rebro12->opposize_pts.insert(p3);
            rebro23->opposize_pts.insert(p1);
            rebro13->opposize_pts.insert(p2);


            pointInfos[p1].add_neighbours({p2,p3});
            pointInfos[p2].add_neighbours({p1,p3});
            pointInfos[p3].add_neighbours({p1,p2});


            std::set<int> s= {p1,p2,p3};
            if(s.size()!=3)
                throw std::runtime_error("if(s.size()!=3)##1");
            triangles.insert(s);
        }

    }

}


void surface::run(const std::string &fn_in, const std::string& fn_out)
{
    load_points(fn_in);

    std::cout << "Calculate triangles" <<std::endl;



    pointInfos.resize(pts.size());
    std::set<int> except;
    int p1=rand()%pts.size();
    std::set<int> s1;
    s1.insert(p1);
    printf("p1 %d\n",p1);

    REF_getter<rebro_container> rebro0=new rebro_container({p1});

//    std::deque<REF_getter<rebro_container> > rebras_copy;

    auto p2=find(pts[p1], rebro0->points,rebro0->opposize_pts,0);
    printf("p2 %d\n",p2);
    if(p2!=-1)
    {
        rebro0->points.insert(p2);
        all_rebras.insert({{p1,p2},rebro0});
        rebras_to_process.push_back(rebro0);
        pointInfos[p1].rebras.insert({rebro0->points,rebro0});
        pointInfos[p2].rebras.insert({rebro0->points,rebro0});
        pointInfos[p1].add_neighbours({p2});
        pointInfos[p2].add_neighbours({p1});
    }
    process_rebras(0,true);
    printf("triangles %d\n",triangles.size());
    printf("second process\n");
    for(auto& z: all_rebras)
    {
        if(z.second->opposize_pts.size()<2)
            rebras_to_process.push_back(z.second);
    }
    process_rebras(100,false);
    printf("triangles %d\n",triangles.size());

//    std::deque<std::set<int> > rebraWith_1_opposite;
    std::map<int,int> opps_for_rebra;
    for(auto&z: all_rebras)
    {
        opps_for_rebra[z.second->opposize_pts.size()]++;
    }
    for(auto& z: opps_for_rebra)
    {
        printf("opps_for_rebra %d %d\n",z.first,z.second);
    }

    for(int i=0;i< pts.size();i++)
    {
        auto& p=pointInfos[i];
        for(auto &r1: p.rebras)
        {
            for(auto& r2: p.rebras)
            {

            }
        }
    }



    std::deque<std::set<int> > added_tri;
    /// connect points with 2 neigbours
    int matchcount=0;



    std::map<int,int> cnt;
    for(auto & z: pointInfos)
    {
        cnt[z.neighbours.size()]++;
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

