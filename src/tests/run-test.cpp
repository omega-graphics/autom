#include "ExecEngine.h"
#include "TargetDumper.h"

#include <fstream>
#include <iomanip>
#include <iostream>

#include "ADT.h"

int main(int argc,char *argv[]){
    
    autom::StrRef file(argv[1]);

    auto t = autom::TargetNinja();

    autom::ExecEngineOpts opts {*t};
    autom::TargetDumper dumper(std::cout);
    autom::ExecEngine execEngine(opts);

    std::ifstream in(file);

    execEngine.parseAndEvaluate(&in);

    return 0;
}