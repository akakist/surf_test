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
    if(rebro.size()!=2)
        throw std::runtime_error("if(rebro.size()!=2)");
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
std::set<int> surface::get_rebro_neighbours(const REF_getter<rebro_container>&r)
{
    std::set<int> ret;
    for(auto& p:r->points)
    {
        pointInfo& pi=pointInfos[p];
        for(auto& r:pi.rebras)
        {
            if(r.second->opposize_pts.size()<2)
            {
                for(auto& z:r.second->points)
                    ret.insert(z);
            }
        }
    }
    return ret;
}
void surface::process_point( const std::vector<int> vp)
{
    for(auto& p1:vp)
    {

    pointInfo &pi1=pointInfos[p1];
    auto unfilled_rebraz=pi1.getUnfilledRebras();
//    printf("unfilled_rebraz.size()==%d\n",unfilled_rebraz.size());
    if(unfilled_rebraz.size()==2)
    {
        auto rebro1=*unfilled_rebraz.begin();
        auto rebro2=*unfilled_rebraz.rbegin();
        int p2=get_rebro_peer(rebro1.second->points,p1);
        int p3=get_rebro_peer(rebro2.second->points,p1);
        auto v2=pts[p2];
        v2.sub(pts[p1]);
        auto v3=pts[p3];
        v3.sub(pts[p1]);
        auto angle=Angle::angle(v2,v3);
//        printf("angle %lf\n",angle* (180/M_PI));
        if(angle< M_PI_2)
        {
            pointInfo &pi2=pointInfos[p2];
            pointInfo &pi3=pointInfos[p3];
            auto rebro3=getRebroOrCreate({p2,p3},"process_point");
            if(rebro3->opposize_pts.size()==2)
                return;
            pi2.add_neighbours({p3});
            pi3.add_neighbours({p2});
            pi2.rebras.insert({{p2,p3},rebro3});
            pi3.rebras.insert({{p2,p3},rebro3});
            rebro1.second->opposize_pts.insert(p3);
            rebro2.second->opposize_pts.insert(p3);
            rebro3->opposize_pts.insert(p1);
            pi1.calcArounded();
            pi2.calcArounded();
            pi3.calcArounded();
            if(pi1.arounded)
                searchSet.erase(p1);
            if(pi2.arounded)
                searchSet.erase(p2);
            if(pi3.arounded)
                searchSet.erase(p3);

            triangles.insert({p1,p2,p3});
            rebras_to_process.insert({{rebro3->points},rebro3});
            rebras_to_process.erase(rebro1.second->points);
            rebras_to_process.erase(rebro2.second->points);
        }

    }
    }
}

void surface::process_rebras( std::set<int>& searchSet,int refcount, bool append_new_rebras)
{
    while(rebras_to_process.size())
    {
//        printf("z\n");

        if(rebras_to_process.size()%100==0)
            printf("rebras remained %d\n",rebras_to_process.size());
        auto rebro12=rebras_to_process.begin()->second;

        rebras_to_process.erase(rebras_to_process.begin());
        if(rebro12->opposize_pts.size()>=2)
            continue;



        int p1=*rebro12->points.begin();
        int p2=*rebro12->points.rbegin();
        pointInfo & pi1=pointInfos[p1];
        pointInfo & pi2=pointInfos[p2];
        if(pi1.rebras.size()>3)
            process_point({p1});
        if(pi2.rebras.size()>3)
            process_point({p2});

        if(rebro12->opposize_pts.size()>=2)
            continue;

        point rc=rebro_center(rebro12);
        auto except=get_rebro_pts(rebro12);
        auto p3=find( rc,except,{},refcount);
//        printf("p3 %d\n",p3);
        if(p3==-1)
        {
            printf("p3 -1 \n");
            rebro12->opposize_pts.insert(-1);
            continue;
        }




        {
            auto d=fdist(pts[p3],rc);
            if(d>101)
            {
                continue;
            }
            std::set<int> s_23({p2,p3});
            std::set<int> s_13({p1,p3});

            REF_getter<rebro_container> rebro23(getRebroOrCreate(s_23,"process rebras"));
            if(rebro23->opposize_pts.size()>1)
                continue;


            REF_getter<rebro_container> rebro13(getRebroOrCreate(s_13,"process rebras"));
            if(rebro13->opposize_pts.size()>1)
                continue;
            pointInfo & pi3=pointInfos[p3];
            pi1.rebras.insert({rebro13->points,rebro13});
            pi1.rebras.insert({rebro12->points,rebro12});

            pi2.rebras.insert({rebro12->points,rebro12});
            pi2.rebras.insert({rebro23->points,rebro23});

            pi3.rebras.insert({rebro13->points,rebro13});
            pi3.rebras.insert({rebro23->points,rebro23});

            rebro12->opposize_pts.insert(p3);
            rebro23->opposize_pts.insert(p1);
            rebro13->opposize_pts.insert(p2);

            pi1.add_neighbours({p2,p3});
            pi2.add_neighbours({p1,p3});
            pi3.add_neighbours({p1,p2});
            pi1.calcArounded();
            pi2.calcArounded();
            pi3.calcArounded();
            if(pi1.arounded)
                searchSet.erase(p1);
            if(pi2.arounded)
                searchSet.erase(p2);
            if(pi3.arounded)
                searchSet.erase(p3);
            process_point({p1,p2,p3});


            if(append_new_rebras)
            {
                rebras_to_process.insert({rebro23->points,rebro23});
                rebras_to_process.insert({rebro13->points,rebro13});
            }





            std::set<int> s= {p1,p2,p3};
            if(s.size()!=3)
                throw std::runtime_error("if(s.size()!=3)##1");
            triangles.insert(s);
        }

    }

}


void surface::run(const std::string &fn_in, const std::string& fn_out)
{
    std::srand(time(NULL));
    load_points(fn_in);

    std::cout << "Calculate triangles" <<std::endl;

    calc_figure_size();

    pointInfos.resize(pts.size());
    std::set<int> except;
//    int p1=rand()%pts.size();
    int p1=0;
    std::set<int> s1;
    s1.insert(p1);
    printf("p1 %d\n",p1);

    for(int i=0;i<pts.size();i++)
    {
        searchSet.insert(i);
    }

    auto p2=find(pts[p1], {p1},{},0);
    printf("p2 %d\n",p2);
    if(p2!=-1)
    {
        REF_getter<rebro_container> rebro0=new rebro_container({p1,p2},"first rebro");
        all_rebras.insert({{p1,p2},rebro0});
        rebras_to_process.insert({{p1,p2},rebro0});
        pointInfos[p1].rebras.insert({rebro0->points,rebro0});
        pointInfos[p2].rebras.insert({rebro0->points,rebro0});
        pointInfos[p1].add_neighbours({p2});
        pointInfos[p2].add_neighbours({p1});
    }

    process_rebras(searchSet,0,true);

    printf("start rm \n");
    bool done=false;
//    while(!done)
//    {
//        std::set<int>rm;
//        for(auto& s:searchSet)
//        {
//            auto& p=pointInfos[s];
//            bool all_r_ok=true;
//            for(auto& r: p.rebras)
//            {
//                if(r.second->opposize_pts.size()!=2)
//                {
//                    rebras_to_process.insert({r.second->points,r.second});
//                    all_r_ok=false;
//                }
//            }
//            if(all_r_ok)
//                rm.insert(s);
//        }
//        for(auto& r:rm)
//        {
//            searchSet.erase(r);
//        }
//        if(rebras_to_process.empty())
//            done=true;
//        process_rebras(searchSet,100,true);
//        printf("start rm size %d\n",rm.size());
//    }

    printf("triangles %d\n",triangles.size());
    printf("second process\n");
//    for(auto& z: all_rebras)
//    {
//        if(z.second->opposize_pts.size()<2)
//            rebras_to_process.push_back(z.second);
//    }
//    process_rebras(100,false);
    printf("triangles %d\n",triangles.size());

//    std::deque<std::set<int> > rebraWith_1_opposite;
    std::map<int,int> opps_for_rebra;
    std::map<const char*,int> byComments;
    for(auto&z: all_rebras)
    {
        opps_for_rebra[z.second->opposize_pts.size()]++;
        byComments[z.second->comment]++;
    }
    for(auto& z: opps_for_rebra)
    {
        printf("opps_for_rebra %d %d\n",z.first,z.second);
    }
    for(auto& z: byComments)
    {
        printf("byComments %s %d\n",z.first,z.second);
    }
    for(int i=0;i<pointInfos.size();i++)
    {
        if(!pointInfos[i].arounded)
        {

        }
    }
    std::map<int,int> arounded;
    for(int i=0;i<pointInfos.size();i++)
    {
        arounded[pointInfos[i].arounded]++;
    }
    for(auto& z: arounded)
    {
        printf("arounded %d %d\n",z.first,z.second);
    }


#ifdef KALL


    for(auto &rebro0: all_rebras)
    {

        if(rebro0.second->opposize_pts.size()==1)
        {

            for(auto& p_common: rebro0.second->points)
            {

//                std::vector<REF_getter<rebro_container> > reb1;
                for(auto& rebro1: pointInfos[p_common].rebras)
                {
                    if(rebro1.second->opposize_pts.size()==1)
                    {

                        int p0=get_rebro_peer(rebro0.second->points,p_common);

                        int p1=get_rebro_peer(rebro1.second->points,p_common);
                        REF_getter<rebro_container> rebro3=getRebroOrCreate({p0,p1});
                        rebro3->opposize_pts.insert(p_common);
                        rebro0.second->opposize_pts.insert(p1);
                        rebro1.second->opposize_pts.insert(p0);
                        rebro3->opposize_pts.insert(p_common);
                        pointInfos[p0].add_neighbours({p1,p_common});
                        pointInfos[p1].add_neighbours({p2,p_common});
                        rebras_to_process.push_back(rebro3);

                    }

                }



//                }
            }
        }
    }
#endif
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

REF_getter<rebro_container> surface::getRebroOrCreate(const std::set<int>& s, const char *comment)
{
    auto i=all_rebras.find(s);
    if(i==all_rebras.end())
    {
            REF_getter<rebro_container >rebro = new rebro_container(s, comment);
            all_rebras.insert({s,rebro});

            return rebro;
    }
    else {
        return i->second;

    }

}
int surface::algoFind__findBrutforce(const point &pt,  const std::set<int> &rebro, const std::set<int> &except_pts, int refcount)
{
    real min_d=std::numeric_limits<double>::max();
    int selected=-1;
//    printf("searchSet size %d\n",searchSet.size());
    for(auto &i:searchSet)
    {
//        for(int i=0;i<pts.size();i++)
        {
            pointInfo pi=pointInfos[i];
            if(pi.arounded)
                continue;
            if(pi.neighbours.size()>3) continue;

//            if(pi.neighbours.size()>refcount)
//            {
//                continue;
//            }

            if(rebro.count(i))
            {
                continue;
            }
            if(except_pts.count(i))
            {
                continue;
            }

            double d=fdist(pt,pts[i]);
            if(d>figure_size/10)
                continue;
            if(d<min_d)
            {


                min_d=d;

                selected=i;
            }

        }
    }
    return selected;
}
int surface::algoFind__findNearest(const point &pt, const std::set<int> &rebro, const std::set<int> &except_pts, int refcount)
{
    if(algoFind.need_rebuild)
    {
        algoFind__rebuild(pt,pts,searchSet);
        algoFind.need_rebuild=false;
    }
    real reper_dist=fdist(pt,algoFind.reper);
    std::set<int> result;
    for(auto it=algoFind.dists.begin();it!=algoFind.dists.end();it++)
    {
        auto& newp=it->second;
        auto& d=it->first;
        if(d>reper_dist && result.size())
            break;
        {
            pointInfo pi=pointInfos[newp];
            if(pi.arounded)
                continue;
            if(pi.neighbours.size()>refcount)
            {
                continue;
            }
            if(rebro.count(newp))
            {
                continue;
            }
            if(except_pts.count(newp))
            {
                continue;
            }


            result.insert(newp);
        }
    }
    real min_d=std::numeric_limits<double>::max();
    int selected=-1;
    for(auto& z: result)
    {
        double d=fdist(pt,pts[z]);
        if(d<min_d)
        {
            min_d=d;
            selected=z;
        }

    }
    if(result.size()>1000)
        algoFind.need_rebuild=true;

    if(selected==-1)
        throw std::runtime_error("if(selected==-1)");
//    printf("selected %d\n",selected);
    return selected;

}
void surface::algoFind__rebuild(const point &p, const std::vector<point >&pts,const std::set<int> &searchSet)
{
    algoFind.dists.clear();
    algoFind.reper=p;
    for(auto& i: searchSet)
    {
        algoFind.dists.insert({fdist(p,pts[i]),i});
    }
}
std::map<std::set<int>,REF_getter<rebro_container>> pointInfo::getUnfilledRebras()
{
    std::map<std::set<int>,REF_getter<rebro_container>> ret;
//    printf("rebras size %d\n",rebras.size());
    for(auto&r: rebras)
    {
        if(r.second->opposize_pts.size()<2)
        {
            ret.insert({r.second->points,r.second});
        }
    }
    return ret;

}
void surface::calc_figure_size()
{
    point min=pts[0];
    point max=pts[0];

     for(size_t i=0; i<pts.size(); i++)
     {
         auto & p=pts[i];
         if(p.x>max.x) max.x=p.x;
         if(p.y>max.y) max.y=p.y;
         if(p.z>max.z) max.z=p.z;

         if(p.x<min.x) min.x=p.x;
         if(p.y<min.y) min.y=p.y;
         if(p.z<min.z) min.z=p.z;

     }

     point d=max;
     d.sub(min);
    figure_size=std::max(d.x,std::max(d.y,d.z));
}
