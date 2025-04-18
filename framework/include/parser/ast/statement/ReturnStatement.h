// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_PARSER_AST_STATEMENT_RETURN_STATEMENT_H
#define BASILISK_FRAMEWORK_PARSER_AST_STATEMENT_RETURN_STATEMENT_H 1

#include "parser/ast/ASTNode.h"

#include <memory>

namespace parser
{
    class ReturnStatement : public ASTNode
    {
    public:
        ReturnStatement(Scope* scope, ASTNodePtr returnValue, SourcePair source);

        virtual vipir::Value* codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag) override;

        virtual void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

    private:
        ASTNodePtr mReturnValue;
    };
    using ReturnStatementPtr = std::unique_ptr<ReturnStatement>;
}

#endif // BASILISK_FRAMEWORK_PARSER_AST_STATEMENT_RETURN_STATEMENT_H