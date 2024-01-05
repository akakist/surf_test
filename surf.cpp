#include "surf.h"
#include "utils.h"
#include <fstream>
#include <iostream>
#include <deque>
#include <json/json.h>
#define MAX_N 20
bool surface::line_len_ok(real len)
{
//    return true;
    return len<picture_size/MAX_N;
}
bool is_triangle_good(const point& p1,const point&  p2, const point&  p3)
{
    return true;
    std::set<real> s;
    s.insert({fdist(p1,p2),fdist(p1,p3),fdist(p2,p3)});
    return *s.rbegin() / *s.begin()<1.3;
}

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
struct compare_stuff
{
    REF_getter<rebro_container> r2;
    REF_getter<rebro_container> r3;
    int peerpoint2,peerpoint3;
    compare_stuff():r2(nullptr),r3(nullptr) {}
};
//std::deque<REF_getter<figure> >
//std::map<int,REF_getter<figure> > figures;
void surface::move_content(const REF_getter<figure>& to, const REF_getter<figure>&from)
{
    for(auto&p: from->points)
    {
        to->points.insert(p);
        to->sum_points+=pts[p];
        pointInfos[p].figure_=to;
    }
    for(auto& r: from->rebras)
    {
        to->rebras.insert({r.first,r.second});
    }
    from->points.clear();
    from->sum_points= {0,0,0};
    from->rebras.clear();
    all_figures.erase(from->id);

}
std::string surface::dump_figure(const REF_getter<figure>&f)
{
    Json::Value j;
//    f->points;
    std::map<int, int> bord;
    j["pcount"]=(int)f->points.size();
    for(auto& p:f->points)
    {
        auto& pi=pointInfos[p];
        bord[pi.can_connected()]++;
    }
    for(auto& z:bord)
    {
        j["bord"][std::to_string(z.first)]=z.second;
    }
    return j.toStyledString();
//    f->
}
void surface::connect_unlinked_points()
{
    std::set<int> to_process;
    std::set<int> seachSet;
    for(int i=0;i<pointInfos.size();i++)
    {
        auto &pi0=pointInfos[i];

        if(!pi0.figure_.valid())
        {
            to_process.insert(i);
        }
        else
        {
            seachSet.insert(i);
        }
    }
//    for(auto& c: to_process)
    {
        for(auto& p: to_process)
        {
            auto min=std::numeric_limits<real>::max();
            int selected=-1;
            for(auto& s:seachSet)
            {
                if(s==p) continue;
    //            if(pil.neighbours.count(s))
    //                continue;
                auto d=fdist(pts[p],pts[s]);
                if(d<min /*&& line_len_ok(d)*/)
                {
                    selected=s;
                    min=d;
                }
            }
            if(selected!=-1)
            {
                auto r=getRebroOrCreate({p,selected},"connect_unlinked");
                auto p0=p;
                auto p1=selected;
                auto &pi0=pointInfos[p];
                auto &pi1=pointInfos[selected];
                pi0.add_neighbours({p1});
                pi1.add_neighbours({p0});
                pi0.add_to_rebras(r);
                pi1.add_to_rebras(r);
                if(!pi1.figure_.valid())
                    throw std::runtime_error("if(!pi1.figure_.valid())");
                pi0.figure_=pi1.figure_;
                pi1.figure_->points.insert(p0);
                pi1.figure_->sum_points+=pts[p];
            }


        }
    }
}
void surface::supress_figures()
{
    std::multimap<int, REF_getter<figure> > sorted_f;
    for(auto& f: all_figures)
    {
        sorted_f.insert({f.second->points.size(),f.second});
    }
    for(auto&f: sorted_f)
    {
        if(f.second->points.size()==0)
            continue;
        auto min=std::numeric_limits<real>::max();
        int selected=-1;
        point p0=f.second->center_point();
        for(int i=0;i<pts.size();i++)
        {
            auto &pi=pointInfos[i];
            if(pi.figure_->id==f.second->id)
                continue;
            auto d=fdist(pts[i],p0);
            if(d<min)
            {
                selected=i;
                min=d;
            }
        }
        if(selected==-1)
            continue;
        auto &pisel=pointInfos[selected];
        auto f2=pisel.figure_;
        if(f.second->points.size()==0)
            throw std::runtime_error("if(f.second->points.size()==0)");
        if(f2.valid() && f.second.valid())
        {
            connect_2_figures(f2,f.second);
//            triangulate_figure(f2);
        }
    }

}
REF_getter<figure> surface::connect_2_figures(const REF_getter<figure>& f1, const REF_getter<figure>& f2)
{
    if(f1->points.size()==0)
        throw std::runtime_error("if(f1->points.size()==0)");
    int n2=get_nearest_point_in_figure(f2,f1->center_point());
    int n1=get_nearest_point_in_figure(f1,pts[n2]);
    if(f1->points.size()>3)
    {
        n2=get_nearest_point_in_figure(f2,pts[n1]);
    }
    if(f2->points.size()>3)
    {
        n1=get_nearest_point_in_figure(f1,pts[n2]);
    }
    auto r=getRebroOrCreate({n1,n2},"connect_2_figures");
    auto &pi1=pointInfos[n1];
    auto &pi2=pointInfos[n2];
    pi1.add_neighbours({n2});
    pi2.add_neighbours({n1});
    pi1.add_to_rebras(r);
    pi2.add_to_rebras(r);
    move_content(f1,f2);
    return f1;
}
//void surface::/*connect_2_figures*/(
std::string dump_set_int(const std::set<int> &s)
{
    std::string out;
    for(auto& z:s)
    {
        out+=std::to_string(z)+" ";
    }
    return out;
}
void surface::proceed_tiangle(int p0, int p2, int p3)
{
    if(!validate_triangle(p0,p2,p3))
        return;
    auto &pi0=pointInfos[p0];
    auto &pi2=pointInfos[p2];
    auto &pi3=pointInfos[p3];
    auto r02=getRebroOrCreate({p0,p2},"link_neighbours2");
    auto r03=getRebroOrCreate({p0,p3},"link_neighbours2");
    auto r23=getRebroOrCreate({p2,p3},"link_neighbours2");

    if(r02->opposize_pts.size()>1 || r03->opposize_pts.size()>1 || r23->opposize_pts.size()>1)
    {
        printf("if(r02->opposize_pts.size()>1 || r03->opposize_pts.size()>1 || r23->opposize_pts.size()>1)\n");
        return;
    }
    pi0.add_to_rebras(r02);
    pi0.add_to_rebras(r03);
    pi2.add_to_rebras(r02);
    pi2.add_to_rebras(r23);
    pi3.add_to_rebras(r03);
    pi3.add_to_rebras(r23);

    pi0.add_neighbours({p2,p3});
    pi2.add_neighbours({p0,p3});
    pi3.add_neighbours({p0,p2});
    r02->add_opposite_pts(p3);
    r03->add_opposite_pts(p2);
    r23->add_opposite_pts(p0);
    if(r02->opposize_pts.size()==2)
    {
        pi0.neighbours.erase(p2);
        pi2.neighbours.erase(p0);
    }
    if(r03->opposize_pts.size()==2)
    {
        pi0.neighbours.erase(p3);
        pi3.neighbours.erase(p0);
    }
    if(r23->opposize_pts.size()==2)
    {
        pi2.neighbours.erase(p3);
        pi3.neighbours.erase(p2);
    }

    if(!triangles.count({p0,p2,p3}))
    {
        printf("1 add triangle %s\n",dump_set_int({p0,p2,p3}).c_str());
        triangles.insert({p0,p2,p3});
    }
    else
        printf("2 triangle already inserted %s\n",dump_set_int({p0,p2,p3}).c_str());


}
bool surface::validate_triangle(int a, int b, int c)
{
//    return true;
    auto d1=fdist(pts[a],pts[b]);
    auto d2=fdist(pts[a],pts[c]);
    auto d3=fdist(pts[b],pts[c]);
    auto max=std::max(d1, std::max(d2,d3));
    auto min=std::min(d1, std::max(d2,d3));
    if(max>min*10)
        return false;
    return true;

}
int surface::find_nearest(const point& pt, const std::set<int> &ps)
{
//    printf("find_nearest sz %d\n",ps.size());
    auto min=std::numeric_limits<real>::max();
    int sel=-1;
    for(auto& p:ps)
    {
        auto d=fdist(pt,pts[p]);
        if(d<min)
        {
            min=d;
            sel=p;
        }
    }
    return sel;
}
void surface::link_neighbours2(const REF_getter<figure>&f)
{
    std::set<int> interested;
    for(auto &p0:f->points)
        interested.insert(p0);
    printf("interested %d\n",interested.size());
    while(interested.size())
    {

        int p0=*interested.begin();
        interested.erase(interested.begin());
//        printf("link_neighbours2\n");
        auto &pi0=pointInfos[p0];
//        std::set<REF_getter<rebro_container> >rs;
        auto nf=pi0.not_filles_rebras();
        printf("pi0.neighbours.size() %d\n",pi0.neighbours.size());
        printf("nf.size() %d\n",nf.size());
/// нужно сначала делать if(pi0.neighbours.size()==1 && nf.size()==1) поскольку иначе идет забивка треугольниками

        if(pi0.neighbours.size()==1 && nf.size()==1)
        {
//            printf("if(pi0.neighbours.size()==1 && nf.size()==1)\n");
            auto p2=*pi0.neighbours.begin();
            auto pi2=pointInfos[p2];
            auto nf2=pi2.not_filles_rebras();
            printf("nf2 size %d\n",nf2.size());
            for(auto& n:nf2)
            {
                if(!n->points.count(p0))
                {
                    auto p3=get_rebro_peer(n,p2);
                    proceed_tiangle(p0,p2,p3);
                }
//                elseprintf("")
            }


        }

        if(pi0.neighbours.size()==2 && nf.size()==2)
        {
            auto ns=pi0.neighbours;
            int p1=*ns.begin();ns.erase(ns.begin());
            int p2=*ns.begin();ns.erase(ns.begin());
            proceed_tiangle(p0,p1,p2);


        }
        if(pi0.neighbours.size()==3)
        {
            auto ns=pi0.neighbours;
            int p1=*ns.begin();ns.erase(ns.begin());
            int p2=*ns.begin();ns.erase(ns.begin());
            int p3=*ns.begin();ns.erase(ns.begin());

            proceed_tiangle(p0,p1,p2);
            proceed_tiangle(p0,p1,p3);
            proceed_tiangle(p0,p2,p3);
            f->points.erase(p0);
            for(auto& n: pi0.neighbours)
            {
                auto pin=pointInfos[n];
                pin.neighbours.erase(p0);
            }

//            continue;
        }
        nf=pi0.not_filles_rebras();
        if(nf.size()==2)
        {
            std::set<int> pt_s;
            for(auto& r:nf)
            {
                for(auto& p:r->points)
                    pt_s.insert(p);
            }
            pt_s.erase(p0);
            if(pt_s.size()!=2)
                throw std::runtime_error("if(pt_s.size()!=2)");
            proceed_tiangle(p0,*pt_s.begin(),*pt_s.rbegin());
//            continue;
        }
        if(pi0.neighbours.size()==0)
        {
            f->points.erase(p0);
//            continue;
        }

        if(pi0.neighbours.size() == nf.size() && pi0.neighbours.size()>3)
        {
//            printf("tyry nf\n");
            int p1=find_nearest(pts[p0],pi0.neighbours);
            if(p1!=-1)
            {
                auto ns=pi0.neighbours;
                ns.erase(p1);
                ns.erase(p0);
                int p2=find_nearest(pts[p1],ns);
                if(p2!=-1)
                {
                    proceed_tiangle(p0,p1,p2);
                }
                else
                    printf("!if(p2!=-1)\n");
                continue;
            }
            else
                printf("!if(p1!=-1)\n");

        }
         nf=pi0.not_filles_rebras();
        printf("L pi0.neighbours.size() %d\n",pi0.neighbours.size());
        printf("L nf.size() %d\n",nf.size());

//        if(pi0.neighbours.size()>3 && nf.size()==0)
//        {
//            f->points.erase(p0);
//            for(auto& n: pi0.neighbours)
//            {
//                auto pin=pointInfos[n];
//                pin.neighbours.erase(p0);
//            }

//        }
    }
    int cnt=0;
    for(auto& p: f->points)
    {
        auto& pi=pointInfos[p];
        if(pi.neighbours.size()==2)
            cnt++;
    }
    printf("fig %d p.neighbours.size()==2 cnt %d\n", f->id,cnt);
}

//    return n_result;
//}
void surface::run(const std::string &fn_in, const std::string& fn_out)
{

    std::srand(time(NULL));

    load_points(fn_in);

    std::cout << "Calculate triangles" <<std::endl;

    calc_picture_size();

    pointInfos.resize(pts.size());

    printf("max line %lf\n",picture_size/MAX_N);
    step1_split_to_pairs();
    connect_unlinked_points();
    supress_figures();
    for(int i=0;i<50;i++)
    {
    for(auto& f:all_figures)
        link_neighbours2(f.second);
    }

printf("figure_count %d\n",all_figures.size());

    printf("all splitted to triangles\n");
//    pointInfo _pi1=pointInfos[p1];

    printf("triangles %d\n",triangles.size());
    printf("all rebvras %d\n",all_rebras.size());

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
//    for(auto& z: border_rebras)
//    {
//        printf("border_rebras %d %d\n",z.first,z.second);
//    }

    std::deque<std::set<int> > added_tri;
    /// connect points with 2 neigbours

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
    point center= {0,0,0};
    for(auto& ss:s)
    {
        center+=pts[ss];
    }
    center/=s.size();

    auto i=all_rebras.find(s);
    if(i==all_rebras.end())
    {
        REF_getter<rebro_container >rebro = new rebro_container(s,center, comment);
        all_rebras.insert({s,rebro});

        return rebro;
    }
    else {
        return i->second;

    }

}
int surface::algoFind__findBrutforce(const std::set<int>& searchSet1,
                                     const point &pt,  const std::set<int> &rebro, const std::set<int> &except_pts, int refcount)
{
    real min_d=std::numeric_limits<double>::max();
    int selected=-1;
    for(auto &i:searchSet1)
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
        if(! line_len_ok(d))
            continue;
        if(d<min_d)
        {

            min_d=d;

            selected=i;
        }

    }
    return selected;
}

void surface::calc_picture_size()
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
    picture_size=std::max(d.x,std::max(d.y,d.z));
}
