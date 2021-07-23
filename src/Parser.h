#include <memory>

#include "parser/Lexer.h"
#include "parser/ASTFactory.h"
#include "parser/Execution.h"

#ifndef AUTOM_PARSER_H
#define  AUTOM_PARSER_H

namespace autom {

    namespace eval {
        class Eval;
    };

    class Parser final {

        std::unique_ptr<Lexer> lexer;

        std::unique_ptr<ASTFactory> astFactory;

        std::unique_ptr<eval::Eval> exec;

    public:
        Parser(TargetConsumer &targetConsumer);
        void parseAndEvaluate(std::istream * in);
        ~Parser();
    };
};

#endif