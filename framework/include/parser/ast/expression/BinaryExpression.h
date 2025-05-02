// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_BINARY_EXPRESSION_H
#define BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_BINARY_EXPRESSION_H 1

#include "parser/ast/ASTNode.h"

#include "lexer/Token.h"

#include <memory>

namespace parser
{
    class BinaryExpression : public ASTNode
    {
    public:
        enum class Operator 
        {
            // Mathematical
            Add,
            Sub,
            Mul,
            Div,

            // Comparison
            Equal,
            NotEqual,
            LessThan,
            GreaterThan,
            LessEqual,
            GreaterEqual,

            // Assignment
            Assign,

            // Special
            Index
        };

        BinaryExpression(Scope* scope, ASTNodePtr left, lexer::Token operatorToken, ASTNodePtr right, SourcePair source);

        virtual vipir::Value* codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag) override;

        virtual std::vector<ASTNode*> getChildren() override;

        virtual void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

    private:
        ASTNodePtr mLeft;
        ASTNodePtr mRight;
        Operator mOperator;
        lexer::Token mOperatorToken;
    };
    using BinaryExpressionPtr = std::unique_ptr<BinaryExpression>;
}

#endif // BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_BINARY_EXPRESSION_H