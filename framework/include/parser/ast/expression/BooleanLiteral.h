// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_BOOLEAN_LITERAL_H
#define BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_BOOLEAN_LITERAL_H 1

#include "parser/ast/ASTNode.h"

#include <cstdint>
#include <memory>

namespace parser
{
    class BooleanLiteral : public ASTNode
    {
    public:
    BooleanLiteral(Scope* scope, bool value, SourcePair source);

        virtual vipir::Value* codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag) override;

        virtual void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;
        
        virtual bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) override;

    private:
        bool mValue;
    };
    using BooleanLiteralPtr = std::unique_ptr<BooleanLiteral>;
}

#endif // BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_BOOLEAN_LITERAL_H