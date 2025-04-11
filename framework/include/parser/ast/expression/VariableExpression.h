// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_VARIABLE_EXPRESSION_H
#define BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_VARIABLE_EXPRESSION_H 1

#include "parser/ast/ASTNode.h"

#include <cstdint>
#include <memory>

namespace parser
{
    class VariableExpression : public ASTNode
    {
    friend class CallExpression;
    public:
        VariableExpression(Scope* scope, std::string name);

        virtual vipir::Value* codegen(vipir::IRBuilder& builder, vipir::Module& module, diagnostic::Diagnostics& diag) override;

    private:
        std::string mName;
    };
    using VariableExpressionPtr = std::unique_ptr<VariableExpression>;
}

#endif // BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_VARIABLE_EXPRESSION_H