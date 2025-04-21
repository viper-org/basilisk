// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_NULLPTR_LITERAL_H
#define BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_NULLPTR_LITERAL_H 1

#include "parser/ast/ASTNode.h"

#include <memory>

namespace parser
{
    class NullptrLiteral : public ASTNode
    {
    public:
        NullptrLiteral(Scope* scope, SourcePair source);

        virtual vipir::Value* codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag) override;

        virtual void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;
        
        virtual bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) override;
    };
    using NullptrLiteralPtr = std::unique_ptr<NullptrLiteral>;
}

#endif // BASILISK_FRAMEWORK_PARSER_AST_EXPRESSION_NULLPTR_LITERAL_H