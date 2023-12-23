
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
    surface s;
    s.run(infile,outfile);

    return 0;
}


