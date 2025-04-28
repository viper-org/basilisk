// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_PARSER_AST_STATEMENT_CONTINUE_STATEMENT_H
#define BASILISK_FRAMEWORK_PARSER_AST_STATEMENT_CONTINUE_STATEMENT_H 1

#include "parser/ast/ASTNode.h"

#include <memory>

namespace parser
{
    class ContinueStatement : public ASTNode
    {
    public:
        ContinueStatement(Scope* scope, SourcePair source);

        virtual vipir::Value* codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag) override;

        virtual void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;
    };
    using ContinueStatementPtr = std::unique_ptr<ContinueStatement>;
}

#endif // BASILISK_FRAMEWORK_PARSER_AST_STATEMENT_CONTINUE_STATEMENT_H