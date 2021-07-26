#include "ExecEngine.h"
#include "TargetDumper.h"

#include <fstream>
#include <iomanip>
#include <iostream>

#include "ADT.h"

int main(int argc,char *argv[]){
    
    autom::StrRef file(argv[1]);

    autom::ExecEngineOpts opts;
    autom::TargetDumper dumper(std::cout);
    autom::ExecEngine execEngine(dumper,opts);

    std::ifstream in(file);

    execEngine.parseAndEvaluate(&in);

    return 0;
}