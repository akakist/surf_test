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
int surface::get_rebro_peer(const REF_getter<rebro_container>& rebro,int n)
{

    if(rebro->points.size()!=2)
        throw std::runtime_error("if(rebro.size()!=2)");
    if(!rebro->points.count(n))
        throw std::runtime_error("if(!rebro.count(n))");
    for(auto& z:rebro->points)
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
void surface::process_point( int p1)
{

    pointInfo &pi1=pointInfos[p1];
    auto border_rebras1=pi1.border_rebras_get();
    if(border_rebras1.size()!=2)
        throw std::runtime_error("if(pi1.border_rebras.size()!=2) "+std::to_string(border_rebras_to_process.size()));

    auto rebro12=*border_rebras1.begin();
    auto rebro13=*border_rebras1.rbegin();

    if(rebro12->opposize_pts.size()==2)
        return;
    if(rebro13->opposize_pts.size()==2)
        return;

    if(rebro12->opposize_pts.size()==2)
    {
        throw std::runtime_error((std::string)"KALL "+__FILE__+" "+std::to_string(__LINE__));
    }
    if(rebro13->opposize_pts.size()==2)
    {
        throw std::runtime_error((std::string)"KALL "+__FILE__+" "+std::to_string(__LINE__));
//        return;
    }
    auto p2=get_rebro_peer(rebro12,p1);
    auto p3=get_rebro_peer(rebro13,p1);


    auto rebro23=getRebroOrCreate({p2,p3},"connect triangle");
    if(rebro23->opposize_pts.size()==2)
        return;


    pointInfo &pi2=pointInfos[p2];
    pointInfo &pi3=pointInfos[p3];

    pi2.add_neighbours({p3});
    pi3.add_neighbours({p2});

    pi2.add_to_rebras(rebro23);
    pi3.add_to_rebras(rebro23);


    rebro12->add_opposite_pts(p3);
    rebro13->add_opposite_pts(p2);
    rebro23->add_opposite_pts(p1);
    border_rebras_to_process.push_back(rebro23);
    triangles.insert({p1,p2,p3});

}


void surface::find_and_add_point_to_rebro(const REF_getter<rebro_container>& rebro12)
{
    {
        int p1=*rebro12->points.begin();
        int p2=*rebro12->points.rbegin();
        pointInfo & pi1=pointInfos[p1];
        pointInfo & pi2=pointInfos[p2];


        point rc=rebro_center(rebro12);
        auto p3=find( rc,{},{},0);
        if(p3==-1)
        {
            printf("p3 -1 \n");
            rebro12->add_opposite_pts(-1);
            return;
        }
        searchSet.erase(p3);
        {
            std::set<int> s_23({p2,p3});
            std::set<int> s_13({p1,p3});

            REF_getter<rebro_container> rebro23(getRebroOrCreate(s_23,"process rebras"));
//            if(rebro23->opposize_pts.size()>1)
//                throw std::runtime_error("if(rebro23->opposize_pts.size()>1)");

//            printf("3 rebro12->opposize_pts.size() %d\n",rebro12->opposize_pts.size());

            REF_getter<rebro_container> rebro13(getRebroOrCreate(s_13,"process rebras"));
//            if(rebro13->opposize_pts.size()>1)
//                throw std::runtime_error("if(rebro23->opposize_pts.size()>1)");

            pointInfo & pi3=pointInfos[p3];

//            printf("4 rebro12->opposize_pts.size() %d\n",rebro12->opposize_pts.size());

            pi1.add_neighbours({p2,p3});
            pi2.add_neighbours({p1,p3});
            pi3.add_neighbours({p1,p2});

//            printf("5 rebro12->opposize_pts.size() %d\n",rebro12->opposize_pts.size());

//            printf("KALL %s %d\n",__FILE__,__LINE__);
            rebro23->add_opposite_pts(p1);
//            printf("KALL %s %d\n",__FILE__,__LINE__);
            rebro13->add_opposite_pts(p2);
//            printf("KALL %s %d\n",__FILE__,__LINE__);
            rebro12->add_opposite_pts(p3);

            pi1.add_to_rebras(rebro13);
            pi2.add_to_rebras(rebro23);
            pi3.add_to_rebras(rebro13);
            pi3.add_to_rebras(rebro23);



            /// global border_rebras


            for(auto& r:{rebro13,rebro23,rebro12})
            {
                if(r->opposize_pts.size()<2)
                    border_rebras_to_process.push_back(r);
            }

            ////
            ///
//            if(rebro12->opposize_pts.size()==2)
//            {
//                pi1.border_rebras.erase(rebro12->points);
//                pi2.border_rebras.erase(rebro12->points);
//            }


//            pi1.add_to_border_rebras(rebro13);

//            pi2.add_to_border_rebras(rebro23);


//            pi3.add_to_border_rebras(rebro13);

//            pi3.add_to_border_rebras(rebro23);

            std::set<int> s= {p1,p2,p3};
            if(s.size()!=3)
                throw std::runtime_error("if(s.size()!=3)##1");
            triangles.insert(s);
        }

    }

}
ABLE surface::can_find_and_add_new(int pt)
{
        pointInfo pi=pointInfos[pt];
        auto border_rebras1=pi.border_rebras_get();
        if(border_rebras1.size()==1)
        {
            /// initial case
            return ABLE::ABLE_FIND_AND_ADD;
        }
        if(border_rebras1.size()!=2)
        {
            return ABLE_SKIP;
            throw std::runtime_error("if(pi.border_rebras.size()!=2) "+std::to_string(border_rebras1.size()));
        }
        if((*border_rebras1.begin())->opposize_pts.size()!=1)
        {
            return ABLE_SKIP;
        }
//            throw std::runtime_error("if(pi.border_rebras.begin()->second->opposize_pts.size()!=1) "+std::to_string(pi.border_rebras.begin()->second->opposize_pts.size()));
        if((*border_rebras1.rbegin())->opposize_pts.size()!=1)
        {
            return ABLE_SKIP;
        }
//            throw std::runtime_error("if(pi.border_rebras.rbegin()->second->opposize_pts.size()!=1) "+std::to_string(pi.border_rebras.rbegin()->second->opposize_pts.size()));

        int p1=pt;
        int p2=get_rebro_peer(*border_rebras1.begin(),p1);
        int p3=get_rebro_peer(*border_rebras1.rbegin(),p1);
        point v1=pts[p1];
        point v2=pts[p2];
        point v3=pts[p3];
        point sum23=((v2-v1)+(v3-v1));
        int op1=*(*border_rebras1.begin())->opposize_pts.begin();
        int op2=*(*border_rebras1.rbegin())->opposize_pts.begin();
        point sumop12=((pts[op1]-v1)+(pts[op2]-v1));
        auto angle=Angle::angle(sum23,sumop12);
        if(angle<60)
        {
            //wypuklost
            return ABLE::ABLE_FIND_AND_ADD;
        }
        else
        {
            // wognutost
            auto angle2=Angle::angle((v2-v1),(v3-v1));
            if(angle2<120)
                return ABLE::ABLE_CONNECT_NEIGHBOUR;

        }
        return ABLE_FIND_AND_ADD;
}
void surface::run(const std::string &fn_in, const std::string& fn_out)
{

    std::srand(time(NULL));

    load_points(fn_in);

    std::cout << "Calculate triangles" <<std::endl;

    calc_figure_size();

    pointInfos.resize(pts.size());

    int p1=0;
    std::set<int> s1;
    s1.insert(p1);
    printf("p1 %d\n",p1);

    for(int i=0;i<pts.size();i++)
    {
        searchSet.insert(i);
    }
    searchSet.erase(p1);
    pointInfo _pi1=pointInfos[p1];
    auto p2=find(pts[p1], {p1},{},0);
    printf("p2 %d\n",p2);
    if(p2!=-1)
    {
        searchSet.erase(p2);
        REF_getter<rebro_container> rebro12=new rebro_container({p1,p2},"first rebro");
        all_rebras.insert({{p1,p2},rebro12});
        pointInfo& _pi2=pointInfos[p2];
        _pi1.add_to_rebras(rebro12);
        _pi2.add_to_rebras(rebro12);

        _pi1.add_neighbours({p2});
        _pi1.add_neighbours({p1});
        point rc=rebro_center(rebro12);
        auto p3=find( rc,{},{},0);
        if(p3!=-1)
        {
            std::set<int> s_23({p2,p3});
            std::set<int> s_13({p1,p3});

            REF_getter<rebro_container> rebro23(getRebroOrCreate(s_23,"process rebras"));
            REF_getter<rebro_container> rebro13(getRebroOrCreate(s_13,"process rebras"));

            pointInfo & _pi3=pointInfos[p3];

//            printf("4 rebro12->opposize_pts.size() %d\n",rebro12->opposize_pts.size());

            _pi1.add_neighbours({p2,p3});
            _pi2.add_neighbours({p1,p3});
            _pi3.add_neighbours({p1,p2});

            rebro23->add_opposite_pts(p1);
            rebro13->add_opposite_pts(p2);
            rebro12->add_opposite_pts(p3);

            _pi1.add_to_rebras(rebro13);
            _pi2.add_to_rebras(rebro23);
            _pi3.add_to_rebras(rebro13);
            _pi3.add_to_rebras(rebro23);



            /// global border_rebras
            for(auto& r:{rebro13,rebro23,rebro12})
            {
                border_rebras_to_process.push_back(r);
            }


            std::set<int> s= {p1,p2,p3};
            if(s.size()!=3)
                throw std::runtime_error("if(s.size()!=3)##1");
            triangles.insert(s);

        }

        while(border_rebras_to_process.size())
        {

            auto rebro=border_rebras_to_process[0];
            border_rebras_to_process.pop_front();

                if(rebro->opposize_pts.size()==2)
                {
                    continue;
                }
                if(rebro->opposize_pts.size()>2)
                    throw std::runtime_error("if(rebro->opposize_pts.size()>2) "+std::to_string(rebro->opposize_pts.size()));

//                if()
                int p1=rebro->left();
                int p2=rebro->right();
                auto a1=can_find_and_add_new(p1);
                auto a2=can_find_and_add_new(p2);
                if(a1==ABLE_SKIP)
                    continue;
                if(a2==ABLE_SKIP)
                    continue;
                if(a1==ABLE_FIND_AND_ADD && a2==ABLE_FIND_AND_ADD)
                {
                    find_and_add_point_to_rebro(rebro);
                }
                else
                {

                    //// add existing point
                    if(a1==ABLE_CONNECT_NEIGHBOUR)
                    {
                        process_point(p1);
                    }
                    else if(a2==ABLE_CONNECT_NEIGHBOUR)
                    {
                        process_point(p2);
                    }
                }

        }
    }


//    process_rebras(searchSet,0,true);

    printf("triangles %d\n",triangles.size());
    printf("triangles %d\n",triangles.size());

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
    for(auto &i:searchSet)
    {

            pointInfo pi=pointInfos[i];

            if(pi.neighbours.size()>refcount)
            {
                continue;
            }

            if(rebro.count(i))
            {
                continue;
            }
            if(except_pts.count(i))
            {
                continue;
            }

            double d=fdist(pt,pts[i]);
            if(d>figure_size/3)
                continue;
            if(d<min_d)
            {


                min_d=d;

                selected=i;
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
