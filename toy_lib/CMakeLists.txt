cmake_minimum_required(VERSION 3.23)

project(toy_lib CXX)

set(PARSER_FILES
    parser/lex.cpp
    parser/lex.hpp
    parser/token.hpp
    parser/parser.cpp
    parser/parser.hpp
    )

set(SEMA_FILES
    sema/sema.cpp
    sema/sema.hpp
    sema/symtab.hpp
    sema/symtab.cpp
    sema/types.hpp
)

set(AST_FILES
    parser/ast/ast.hpp
    parser/ast/visitor.hpp
    parser/ast/base.hpp
    parser/ast/expr.hpp
    parser/ast/expr.cpp
    parser/ast/stmt.hpp
    parser/ast/stmt.cpp
    parser/ast/decl.hpp
    parser/ast/decl.cpp
    parser/ast/builder.hpp
    parser/ast/builder.cpp
)

set(EVAL_FILES
    eval/value.hpp
    eval/value.cpp
    eval/evaluator.cpp
    eval/evaluator.hpp
    eval/value_map.hpp
    eval/value_map.cpp
)

set(STUFF
    stuff/grammar.txt
)

add_library(toy_lib STATIC pch.h ${PARSER_FILES} ${AST_FILES} ${SEMA_FILES} ${EVAL_FILES} ${STUFF})

target_include_directories(toy_lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_features(toy_lib PRIVATE ${OPT_STD})

set_target_properties(toy_lib PROPERTIES
                      VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:toy>")

target_precompile_headers(toy_lib PRIVATE pch.h)

source_group(parser FILES ${PARSER_FILES})
source_group(parser/ast FILES ${AST_FILES})
source_group(sema FILES ${SEMA_FILES})
source_group(eval FILES ${EVAL_FILES})
source_group(stuff FILES ${STUFF})