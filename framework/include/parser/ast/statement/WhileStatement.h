// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_PARSER_AST_STATEMENT_WHILE_STATEMENT_H
#define BASILISK_FRAMEWORK_PARSER_AST_STATEMENT_WHILE_STATEMENT_H 1

#include "parser/ast/ASTNode.h"

#include <memory>

namespace parser
{
    class WhileStatement : public ASTNode
    {
    public:
        WhileStatement(ASTNodePtr condition, ASTNodePtr body, Scope* scope, SourcePair source);

        virtual vipir::Value* codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag) override;

        virtual std::vector<ASTNode*> getChildren() override;

        virtual void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

    private:
        ASTNodePtr mCondition;
        ASTNodePtr mBody;
    };
    using WhileStatementPtr = std::unique_ptr<WhileStatement>;
}

#endif // BASILISK_FRAMEWORK_PARSER_AST_STATEMENT_WHILE_STATEMENT_H