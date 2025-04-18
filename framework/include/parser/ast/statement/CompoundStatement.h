// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_PARSER_AST_STATEMENT_COMPOUND_STATEMENT_H
#define BASILISK_FRAMEWORK_PARSER_AST_STATEMENT_COMPOUND_STATEMENT_H 1

#include "parser/ast/ASTNode.h"

#include <memory>

namespace parser
{
    class CompoundStatement : public ASTNode
    {
    public:
        CompoundStatement(ScopePtr ownScope, std::vector<ASTNodePtr> body, SourcePair source);

        virtual vipir::Value* codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag) override;

        virtual void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

    private:
        std::vector<ASTNodePtr> mBody;

        ScopePtr mOwnScope;
    };
    using CompoundStatementPtr = std::unique_ptr<CompoundStatement>;
}

#endif // BASILISK_FRAMEWORK_PARSER_AST_STATEMENT_COMPOUND_STATEMENT_H