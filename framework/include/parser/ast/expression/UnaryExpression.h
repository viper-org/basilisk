// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_UNARY_EXPRESSION_H
#define BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_UNARY_EXPRESSION_H 1

#include "parser/ast/ASTNode.h"

#include "lexer/Token.h"

#include <memory>

namespace parser
{
    class UnaryExpression : public ASTNode
    {
    public:
        enum class Operator 
        {
            // Special
            AddressOf,
            Indirection
        };

        UnaryExpression(Scope* scope, ASTNodePtr operand, lexer::Token operatorToken, SourcePair source);

        virtual vipir::Value* codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag) override;

        virtual void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

    private:
        ASTNodePtr mOperand;
        Operator mOperator;
        lexer::Token mOperatorToken;
    };
    using UnaryExpressionPtr = std::unique_ptr<UnaryExpression>;
}

#endif // BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_UNARY_EXPRESSION_H