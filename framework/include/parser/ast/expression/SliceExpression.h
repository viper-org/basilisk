// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_SLICE_EXPRESSION_H
#define BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_SLICE_EXPRESSION_H 1

#include "parser/ast/ASTNode.h"

namespace parser
{
    class SliceExpression : public ASTNode
    {
    public:
        SliceExpression(Scope* scope, ASTNodePtr slicee, ASTNodePtr start, ASTNodePtr end, SourcePair source);

        virtual vipir::Value* codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag) override;

        virtual std::vector<ASTNode*> getChildren() override;

        virtual void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

    private:
        ASTNodePtr mSlicee;
        ASTNodePtr mStart;
        ASTNodePtr mEnd;
    };
    using SliceExpressionPtr = std::unique_ptr<SliceExpression>;
}

#endif // BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_SLICE_EXPRESSION_H