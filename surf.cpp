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
void surface::run(const std::string &fn_in, const std::string& fn_out)
{
    load_points(fn_in);
    std::cout << "Calculate reperz" << std::endl;
    calculateReperz();
    std::cout << "done reperz" << std::endl;

    std::cout << "Calculate triangles" <<std::endl;

    std::deque<REF_getter<rebro_container> > rebras;
    std::map<std::set<int>,REF_getter<rebro_container> > all_rebras;
    std::set<std::set<int> > triangles;
    std::map<int,std::set<REF_getter<rebro_container>> > ptIncludesRebras;

    drawedNeighbours4Pt.resize(pts.size());
    pointInfos.resize(pts.size());
    std::set<int> except;
    int p1=rand()%pts.size();
    std::set<int> s1;
    s1.insert(p1);
    printf("p1 %d\n",p1);

    REF_getter<rebro_container> rebro0=new rebro_container({p1});

//    std::deque<REF_getter<rebro_container> > rebras_copy;

    auto p2=algoFind.findNearest(pts[p1],pts,drawedNeighbours4Pt, rebro0->points,rebro0->opposize_pts,0);
    printf("p2 %d\n",p2);
    if(p2!=-1)
    {
        rebro0->points.insert(p2);
        all_rebras.insert({{p1,p2},rebro0});
        rebras.push_back(rebro0);
        ptIncludesRebras[p1].insert(rebro0);
        ptIncludesRebras[p2].insert(rebro0);
    }
    while(rebras.size())
    {
        auto rebro12=rebras[0];
        rebras.pop_front();
        if(rebro12->opposize_pts.size()==2)
            continue;

        int p1=*rebro12->points.begin();
        int p2=*rebro12->points.rbegin();

        point rc=rebro_center(rebro12);

        auto p3=algoFind.findNearest(rc,pts,drawedNeighbours4Pt,rebro12->points,rebro12->opposize_pts,0);//find_1_NearestByReperz(rc,rebro12->points,rebro12->opposize_pts,0);
        printf("p3 %d\n",p3);


//        if(_p3.size())
        {

//            auto p3=*_p3.begin();
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
                    ptIncludesRebras[p2].insert(rebro23);
                    ptIncludesRebras[p3].insert(rebro23);
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
                    ptIncludesRebras[p1].insert(rebro13);
                    ptIncludesRebras[p3].insert(rebro13);
                }
            }
            rebras.push_back(rebro23);
            rebras.push_back(rebro13);
//            rebras_copy.push_back(rebro13);

            rebro12->opposize_pts.insert(p3);
            rebro23->opposize_pts.insert(p1);
            rebro13->opposize_pts.insert(p2);

//            if(rebro12->opposize_pts.size()<2)
//                rebras.push_back(rebro12);
//            if(rebro13->opposize_pts.size()<2)
//                rebras.push_back(rebro13);
//            if(rebro23->opposize_pts.size()<2)
//                rebras.push_back(rebro23);

//            except.insert(p1);
//            except.insert(p2);
//            except.insert(p3);

            drawedNeighbours4Pt[p1].insert(p2);
            drawedNeighbours4Pt[p1].insert(p3);
            drawedNeighbours4Pt[p2].insert(p1);
            drawedNeighbours4Pt[p2].insert(p3);
            drawedNeighbours4Pt[p3].insert(p1);
            drawedNeighbours4Pt[p3].insert(p2);
//            removePointFromDistPtsToRepers({p1,p2,p3});
            std::set<int> s= {p1,p2,p3};
            if(s.size()!=3)
                throw std::runtime_error("if(s.size()!=3)##1");
            triangles.insert(s);
            printf("triangles size %d\n",triangles.size());

        }
//        else continue;

    }
//    std::deque<std::set<int> > rebraWith_1_opposite;

#ifdef KALL
    for(int i=0; i<rebras_copy.size(); i++)
    {
        auto& r=rebras_copy[i];
        ptIncludesRebras[*r.points.begin()].insert(i);
        ptIncludesRebras[*r.points.rbegin()].insert(i);
    }
#endif

//    for(auto& z:opposite_pt_to_rebro)
//    {
//        if(z.second.size()==1)
//        {
//            auto rebro=z.first;

////            auto p1=*rebro.begin();
////            auto p2=*rebro.rbegin();

////            point rc=rebro_center(rebro);
////            auto _p3=find_1_NearestByReperz(rc,rebro,except,0);
//        }
//    }


    std::deque<std::set<int> > added_tri;
    /// connect points with 2 neigbours
    int matchcount=0;
#ifdef KALL
    for(int i=0; i<drawedNeighbours4Pt.size(); i++)
    {
        if(drawedNeighbours4Pt[i].size()==2)
        {
            auto& rsIdx=ptIncludesRebras[i];

            for(auto& rIdx:rsIdx)
            {
                auto rebro=rebras_copy[rIdx];
                int peer=get_rebro_peer(rebro.points,i);
                auto& rs2idx=ptIncludesRebras[peer];
                bool matched=false;
                for(auto& r2idx:rs2idx)
                {
                    auto rebro2=rebras_copy[r2idx];
                    int peer2=get_rebro_peer(rebro2.points,peer);
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
                        auto rebro2=rebras_copy[r2idx];
                        int peer2=get_rebro_peer(rebro2.points,peer);
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
#endif
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
    std::cout << "found cnt " << found_cnt << std::endl;
    std::cout << "not found cnt " << not_found_cnt << std::endl;
    std::cout << "result_empty " << result_empty << std::endl;

}

void surface::calculateReperz()
{
    point min=pts[0];
    point max=pts[0];

    point avg;
    for(size_t i=0; i<pts.size(); i++)
    {
        auto & p=pts[i];
//        avg.add(+=pts[i];
        if(p.x>max.x) max.x=p.x;
        if(p.y>max.y) max.y=p.y;
        if(p.z>max.z) max.z=p.z;

        if(p.x<min.x) min.x=p.x;
        if(p.y<min.y) min.y=p.y;
        if(p.z<min.z) min.z=p.z;

    }

    point d=max;
    d.sub(min);
    real N=2;

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

