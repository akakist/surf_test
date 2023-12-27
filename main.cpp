
#include <string>
#include <map>
#include <set>
#include <algorithm>
#include "utils.h"
#include "point.h"
#include "surf.h"

int main(int argc, char *argv[])
{

    std::string infile,outfile;
    for(int i=1; i<argc-1; i++)
    {
        if((std::string)argv[i]=="-i" ||(std::string)argv[i]=="--input")
            infile=argv[i+1];
        if((std::string)argv[i]=="-o" ||(std::string)argv[i]=="--output")
            outfile=argv[i+1];
    }
    if(infile.size()==0 || outfile.size()==0)
        throw std::runtime_error("infile or outfile not specified");
    surface s;
    s.run(infile,outfile);

    return 0;
}
