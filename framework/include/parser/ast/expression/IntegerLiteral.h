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
        IntegerLiteral(std::uintmax_t value);

        virtual vipir::Value* codegen(vipir::IRBuilder& builder, vipir::Module& module) override;

        // TODO: Add typechecking and casting

    private:
        std::uintmax_t mValue;
    };
    using IntegerLiteralPtr = std::unique_ptr<IntegerLiteral>;
}

#endif // BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_INTEGER_LITERAL_H