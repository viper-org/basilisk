// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_PARSER_AST_AST_NODE_H
#define BASILISK_FRAMEWORK_PARSER_AST_AST_NODE_H 1

#include "scope/Scope.h"

#include "lexer/Token.h"

#include "diagnostic//Diagnostic.h"

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

        ASTNode(Scope* scope, Type* type = nullptr) : mScope(scope), mType(type) {}
        virtual ~ASTNode() { }

        virtual vipir::Value* codegen(vipir::IRBuilder& builder, vipir::Module& module, diagnostic::Diagnostics& diag) = 0;

        // TODO: Add typechecking and casting stuff
        
    protected:
        Scope* mScope;
        Type* mType;
    };
    using ASTNodePtr = std::unique_ptr<ASTNode>;

}

#endif // BASILISK_FRAMEWORK_PARSER_AST_AST_NODE_H