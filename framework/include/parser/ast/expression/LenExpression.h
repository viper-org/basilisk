// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_LEN_EXPRESSION_H
#define BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_LEN_EXPRESSION_H 1

#include "parser/ast/ASTNode.h"

#include <memory>

namespace parser
{
    class LenExpression : public ASTNode
    {
    public:
        LenExpression(Scope* scope, ASTNodePtr operand, SourcePair source);

        virtual vipir::Value* codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag) override;

        virtual std::vector<ASTNode*> getChildren() override;

        virtual void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;
        virtual bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) override;

    private:
        ASTNodePtr mOperand;
    };
    using LenExpressionPtr = std::unique_ptr<LenExpression>;
}

#endif // BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_LEN_EXPRESSION_H