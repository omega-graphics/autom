#include <memory>

#include "engine/Lexer.h"
#include "engine/ASTFactory.h"
#include "engine/Execution.h"
#include "Gen.h"
#include "TargetDumper.h"

#ifndef AUTOM_PARSER_H
#define  AUTOM_PARSER_H

namespace autom {

    namespace eval {
        class Eval;
    };

    struct ExecEngineOpts {
        Gen &gen;
        std::vector<StrRef> interfaceSearchPaths;
    };

    class ExecEngine final {

        std::unique_ptr<Lexer> lexer;

        std::unique_ptr<ASTFactory> astFactory;

        std::unique_ptr<eval::Eval> exec;

    public:
        ExecEngineOpts & opts;

        ExecEngine(ExecEngineOpts &opts);
        void parseAndEvaluate(std::istream * in);
        bool checkDependencyTree();
        void generate();
        ~ExecEngine();
    };
};

#endif