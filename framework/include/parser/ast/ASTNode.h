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
    struct SourcePair
    {
        lexer::SourceLocation start;
        lexer::SourceLocation end;
    };

    class ASTNode
    {
    friend struct ::ASTNodeIntrospector;
    public:
        using ASTNodePtr = std::unique_ptr<ASTNode>;

        ASTNode(Scope* scope, SourcePair source, Type* type = nullptr) : mScope(scope), mType(type), mSource(source) {}
        virtual ~ASTNode() { }

        Type* getType() { return mType; }
        SourcePair getSourcePair() { return mSource; }

        virtual vipir::Value* codegen(vipir::IRBuilder& builder, vipir::Module& module, diagnostic::Diagnostics& diag) = 0;

        virtual void typeCheck(diagnostic::Diagnostics& diag, bool& exit) = 0;

        virtual bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) { return false; }
        bool canImplicitCast(diagnostic::Diagnostics& diag, Type* destType);

        static ASTNodePtr Cast(ASTNodePtr& node, Type* destType);
        
    protected:
        Scope* mScope;
        Type* mType;
        SourcePair mSource;
    };
    using ASTNodePtr = std::unique_ptr<ASTNode>;

}

#endif // BASILISK_FRAMEWORK_PARSER_AST_AST_NODE_H