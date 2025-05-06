// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_PARSER_AST_AST_NODE_H
#define BASILISK_FRAMEWORK_PARSER_AST_AST_NODE_H 1

#include "scope/Scope.h"

#include "diagnostic//Diagnostic.h"

#include "debug/SourcePair.h"

#include <vipir/IR/IRBuilder.h>
#include <vipir/DI/DIBuilder.h>

#include <cassert>
#include <memory>

namespace parser
{
    class ASTNode
    {
    public:
        using ASTNodePtr = std::unique_ptr<ASTNode>;

        ASTNode(Scope* scope, SourcePair source, Type* type = nullptr) : mScope(scope), mType(type), mSource(source) {}
        virtual ~ASTNode() { }

        Type* getType() { return mType; }
        SourcePair getSourcePair() { return mSource; }

        virtual vipir::Value* codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag) = 0;
        vipir::Value* dcodegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag)
        {
            if (mScope->parent) // Don't emit debug info for global scope
                builder.CreateDebugInfo(mSource.start.line, mSource.start.col);
            auto val = codegen(builder, diBuilder, module, diag);
            return val;
        }

        virtual std::vector<ASTNode*> getChildren() { return {}; }

        virtual void typeCheck(diagnostic::Diagnostics& diag, bool& exit) = 0;

        virtual bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) { return false; }
        bool canImplicitCast(diagnostic::Diagnostics& diag, Type* destType);

        virtual ASTNodePtr cloneExternal(Scope* in) { return nullptr; }

        static ASTNodePtr Cast(ASTNodePtr& node, Type* destType);
        
    protected:
        Scope* mScope;
        Type* mType;
        SourcePair mSource;
    };
    using ASTNodePtr = std::unique_ptr<ASTNode>;

}

#endif // BASILISK_FRAMEWORK_PARSER_AST_AST_NODE_H