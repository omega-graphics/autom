#include <memory>

#include "engine/Lexer.h"
#include "engine/ASTFactory.h"
#include "engine/Execution.h"

#ifndef AUTOM_PARSER_H
#define  AUTOM_PARSER_H

namespace autom {

    namespace eval {
        class Eval;
    };

    struct ExecEngineOpts {
        std::vector<StrRef> interfaceSearchPaths;
    };

    class ExecEngine final {

        ExecEngineOpts & opts;

        std::unique_ptr<Lexer> lexer;

        std::unique_ptr<ASTFactory> astFactory;

        std::unique_ptr<eval::Eval> exec;

    public:
        ExecEngine(TargetConsumer &targetConsumer,ExecEngineOpts & opts);
        void parseAndEvaluate(std::istream * in);
        ~ExecEngine();
    };
};

#endif