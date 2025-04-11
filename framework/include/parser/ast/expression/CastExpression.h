// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_CAST_EXPRESSION_H
#define BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_CAST_EXPRESSION_H 1

#include "parser/ast/ASTNode.h"

#include <cstdint>
#include <memory>

namespace parser
{
    class CastExpression : public ASTNode
    {
    public:
        CastExpression(Scope* scope, ASTNodePtr value, Type* destType);

        virtual vipir::Value* codegen(vipir::IRBuilder& builder, vipir::Module& module, diagnostic::Diagnostics& diag) override;

        virtual void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;
        virtual bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) override;

    private:
        ASTNodePtr mValue;
    };
    using CastExpressionPtr = std::unique_ptr<CastExpression>;
}

#endif // BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_CAST_EXPRESSION_H