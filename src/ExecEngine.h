#include <memory>

#include "engine/Lexer.h"
#include "engine/ASTFactory.h"
#include "engine/Execution.h"
#include "Gen.h"
#include "TargetDumper.h"
#include "Toolchain.h"

#ifndef AUTOM_PARSER_H
#define  AUTOM_PARSER_H

namespace autom {

    namespace eval {
        class Eval;
    };

    struct ExecEngineOpts {
        MapRef<std::string,eval::Object *> globalVars;
        autom::StrRef outputDir;
        Gen &gen;
        autom::StrRef toolchainFile;
        ArrayRef<StrRef> interfaceSearchPaths;
    };

    class ExecEngine final {
        
        unsigned totalTargets = 0;

        std::shared_ptr<Toolchain> toolchain;

        std::unique_ptr<Lexer> lexer;

        std::unique_ptr<ASTFactory> astFactory;

        std::unique_ptr<eval::Eval> exec;

    public:
        void printError(const std::string& msg);
        ExecEngineOpts & opts;
        OutputTargetOpts & outputTargetOpts;

        explicit ExecEngine(ExecEngineOpts &opts,OutputTargetOpts & outputTargetOpts);
        bool parseAndEvaluate(std::istream * in);
        unsigned resetASTFactoryTokenIndex(unsigned new_value);
        std::vector<Tok> * resetASTFactoryTokenVector(std::vector<Tok> *new_vec);
        bool checkDependencyTree();
        void generate();
        void report();
        ~ExecEngine();
    };
};

#endif
