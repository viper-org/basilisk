cmake_minimum_required(VERSION 3.8)

include(FetchContent)
FetchContent_Declare(vipir
    GIT_REPOSITORY https://github.com/viper-org/vipir
    GIT_TAG master
)
FetchContent_MakeAvailable(vipir)

FetchContent_Declare(vlex
    GIT_REPOSITORY https://github.com/viper-org/vlex
    GIT_TAG master)
FetchContent_MakeAvailable(vlex)

set(SOURCES
    "source/parser/Parser.cpp"
    "source/parser/ast/ASTNode.cpp"
    "source/parser/ast/global/Function.cpp"
    "source/parser/ast/statement/ReturnStatement.cpp"
    "source/parser/ast/statement/VariableDeclaration.cpp"
    "source/parser/ast/statement/IfStatement.cpp"
    "source/parser/ast/statement/CompoundStatement.cpp"
    "source/parser/ast/expression/IntegerLiteral.cpp"
    "source/parser/ast/expression/VariableExpression.cpp"
    "source/parser/ast/expression/CastExpression.cpp"
    "source/parser/ast/expression/BinaryExpression.cpp"
    "source/parser/ast/expression/CallExpression.cpp"

    "source/diagnostic/Diagnostic.cpp"

    "source/scope/Scope.cpp"

    "source/type/Type.cpp"
    "source/type/IntegerType.cpp"
    "source/type/VoidType.cpp"
    "source/type/BooleanType.cpp"
    "source/type/FunctionType.cpp"
    "source/type/ErrorType.cpp"
)

set(HEADERS
    "include/parser/Parser.h"
    "include/parser/ast/ASTNode.h"
    "include/parser/ast/global/Function.h"
    "include/parser/ast/statement/ReturnStatement.h"
    "include/parser/ast/statement/VariableDeclaration.h"
    "include/parser/ast/statement/IfStatement.h"
    "include/parser/ast/statement/CompoundStatement.h"
    "include/parser/ast/expression/IntegerLiteral.h"
    "include/parser/ast/expression/VariableExpression.h"
    "include/parser/ast/expression/CastExpression.h"
    "include/parser/ast/expression/BinaryExpression.h"
    "include/parser/ast/expression/CallExpression.h"

    "include/diagnostic/Diagnostic.h"

    "include/scope/Scope.h"

    "include/type/Type.h"
    "include/type/IntegerType.h"
    "include/type/VoidType.h"
    "include/type/BooleanType.h"
    "include/type/FunctionType.h"
    "include/type/ErrorType.h"
)

source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${SOURCES} ${HEADERS})

add_library(basilisk-framework-basilisk-framework ${SOURCES} ${HEADERS})
add_library(basilisk::framework ALIAS basilisk-framework-basilisk-framework)

target_include_directories(basilisk-framework-basilisk-framework PUBLIC include)
target_compile_features(basilisk-framework-basilisk-framework PUBLIC cxx_std_20)
target_link_libraries(basilisk-framework-basilisk-framework vipir)

vlex_generate(basilisk-framework-basilisk-framework source/lexer/Lexer.vlex source/lexer include/lexer)