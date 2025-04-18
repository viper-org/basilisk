// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_STRING_LITERAL_H
#define BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_STRING_LITERAL_H 1

#include "parser/ast/ASTNode.h"

namespace parser
{
    class StringLiteral : public ASTNode
    {
    public:
        StringLiteral(Scope* scope, std::string value, SourcePair source);

        virtual vipir::Value* codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag) override;

        virtual void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

    private:
        std::string mValue;
    };
    using StringLiteralPtr = std::unique_ptr<StringLiteral>;
}

#endif // BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_STRING_LITERAL_H