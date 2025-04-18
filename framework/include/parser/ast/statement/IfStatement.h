// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_PARSER_AST_STATEMENT_IF_STATEMENT_H
#define BASILISK_FRAMEWORK_PARSER_AST_STATEMENT_IF_STATEMENT_H 1

#include "parser/ast/ASTNode.h"

#include <memory>

namespace parser
{
    class IfStatement : public ASTNode
    {
    public:
        IfStatement(ASTNodePtr condition, ASTNodePtr body, ASTNodePtr elseBody, Scope* scope, SourcePair source);

        virtual vipir::Value* codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag) override;

        virtual void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

    private:
        ASTNodePtr mCondition;
        ASTNodePtr mBody;
        ASTNodePtr mElseBody;
    };
    using IfStatementPtr = std::unique_ptr<IfStatement>;
}

#endif // BASILISK_FRAMEWORK_PARSER_AST_STATEMENT_IF_STATEMENT_H