// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_PARSER_AST_STATEMENT_FOR_STATEMENT_H
#define BASILISK_FRAMEWORK_PARSER_AST_STATEMENT_FOR_STATEMENT_H 1

#include "parser/ast/ASTNode.h"

#include <memory>

namespace parser
{
    class ForStatement : public ASTNode
    {
    public:
        ForStatement(ASTNodePtr init, ASTNodePtr condition, ASTNodePtr it, ASTNodePtr body, Scope* scope, std::string label, SourcePair source);

        virtual vipir::Value* codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag) override;

        virtual void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

    private:
        ASTNodePtr mInit;
        ASTNodePtr mCondition;
        ASTNodePtr mIt;
        ASTNodePtr mBody;

        std::string mLabel;
    };
    using ForStatementPtr = std::unique_ptr<ForStatement>;
}

#endif // BASILISK_FRAMEWORK_PARSER_AST_STATEMENT_FOR_STATEMENT_H