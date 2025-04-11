// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_PARSER_AST_AST_NODE_H
#define BASILISK_FRAMEWORK_PARSER_AST_AST_NODE_H 1

#include "lexer/Token.h"

#include <vipir/IR/IRBuilder.h>

#include <cassert>
#include <memory>

struct ASTNodeIntrospector;

namespace parser
{
    class ASTNode
    {
    friend struct ::ASTNodeIntrospector;
    public:
        using ASTNodePtr = std::unique_ptr<ASTNode>;

        ASTNode() {}
        virtual ~ASTNode() { }

        virtual vipir::Value* codegen(vipir::IRBuilder& builder, vipir::Module& module) = 0;

        // TODO: Add typechecking and casting stuff
    };
    using ASTNodePtr = std::unique_ptr<ASTNode>;

}

#endif // BASILISK_FRAMEWORK_PARSER_AST_AST_NODE_H