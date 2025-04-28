// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_PARSER_AST_STATEMENT_BREAK_STATEMENT_H
#define BASILISK_FRAMEWORK_PARSER_AST_STATEMENT_BREAK_STATEMENT_H 1

#include "parser/ast/ASTNode.h"

#include <memory>

namespace parser
{
    class BreakStatement : public ASTNode
    {
    public:
        BreakStatement(Scope* scope, SourcePair source);

        virtual vipir::Value* codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag) override;

        virtual void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;
    };
    using BreakStatementPtr = std::unique_ptr<BreakStatement>;
}

#endif // BASILISK_FRAMEWORK_PARSER_AST_STATEMENT_BREAK_STATEMENT_H