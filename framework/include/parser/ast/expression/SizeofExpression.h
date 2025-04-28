// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_SIZEOF_EXPRESSION_H
#define BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_SIZEOF_EXPRESSION_H 1

#include "parser/ast/ASTNode.h"

#include <memory>

namespace parser
{
    class SizeofExpression : public ASTNode
    {
    public:
        SizeofExpression(Scope* scope, ASTNodePtr operand, SourcePair source);
        SizeofExpression(Scope* scope, Type* operand, SourcePair source);

        virtual vipir::Value* codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag) override;

        virtual void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;
        
        virtual bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) override;

    private:
        ASTNodePtr mOperandExpression;
        Type* mOperandType;
    };
    using SizeofExpressionPtr = std::unique_ptr<SizeofExpression>;
}

#endif // BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_SIZEOF_EXPRESSION_H