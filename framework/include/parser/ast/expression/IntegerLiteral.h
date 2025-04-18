// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_INTEGER_LITERAL_H
#define BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_INTEGER_LITERAL_H 1

#include "parser/ast/ASTNode.h"

#include <cstdint>
#include <memory>

namespace parser
{
    class IntegerLiteral : public ASTNode
    {
    public:
        IntegerLiteral(Scope* scope, std::uintmax_t value, SourcePair source);

        virtual vipir::Value* codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag) override;

        virtual void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;
        
        virtual bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) override;

    private:
        std::uintmax_t mValue;
    };
    using IntegerLiteralPtr = std::unique_ptr<IntegerLiteral>;
}

#endif // BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_INTEGER_LITERAL_H