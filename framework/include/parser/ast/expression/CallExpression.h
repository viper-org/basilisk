// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_CALL_EXPRESSION_H
#define BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_CALL_EXPRESSION_H 1

#include "parser/ast/ASTNode.h"

#include <cstdint>
#include <memory>

namespace parser
{
    class CallExpression : public ASTNode
    {
    public:
        CallExpression(Scope* scope, ASTNodePtr callee, std::vector<ASTNodePtr> parameters, SourcePair source);

        virtual vipir::Value* codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag) override;

        virtual void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

    private:
        ASTNodePtr mCallee;
        std::vector<ASTNodePtr> mParameters;
        Symbol* mFunction;
    };
    using CallExpressionPtr = std::unique_ptr<CallExpression>;
}

#endif // BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_CALL_EXPRESSION_H