#include "utils.h"
#include <stdio.h>
#include <map>
#include <set>
#include <vector>
std::string loadFile(const std::string& name)
{
    FILE *f = fopen(name.c_str(), "rb");
    if (f==nullptr)
    {
        printf("fopen error: %s\n",name.c_str());
        return "";
    }
    auto res=fseek(f,0,SEEK_END);
    if(res!=0)
    {
        fclose(f);
        printf("fseek error %s\n",name.c_str());
        return "";
    }
    long fsize=ftell(f);
    if(fsize==-1)
    {
        fclose(f);
        printf("ftell error %s\n",name.c_str());
        return "";

    }
    auto res2=fseek(f,0,SEEK_SET);
    if(res2!=0)
    {
        fclose(f);
        printf("fseek error 2 %s\n",name.c_str());
        return "";
    }

    st_malloc buf(fsize);
    auto fres=fread(buf.buf,fsize,1,f);
    if(fres!=1)
    {
        fclose(f);
        printf("fread error %s\n",name.c_str());
        return "";

    }
    fclose(f);
    return  std::string((char*)buf.buf,fsize);

}
std::vector<std::string> splitString(const char *seps, const std::string & src)
{


    std::vector < std::string> res;
    std::set<char>mm;
    size_t l;
    l =::strlen(seps);
    for (unsigned int i = 0; i < l; i++)
    {
        mm.insert(seps[i]);
    }
    std::string tmp;
    l = src.size();

    for (unsigned int i = 0; i < l; i++)
    {

        if (!mm.count(src[i]))
            tmp += src[i];
        else
        {
            if (tmp.size())
            {
                res.push_back(tmp);
                tmp.clear();
            }
        }
    }

    if (tmp.size())
    {
        res.push_back(tmp);
        tmp.clear();
    }
    return res;
}
