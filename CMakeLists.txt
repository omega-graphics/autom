project(autom C CXX)

add_executable("autom" cc/autom.c cc/autom_lexer.h cc/autom_lexer.c cc/autom_parser.cpp)