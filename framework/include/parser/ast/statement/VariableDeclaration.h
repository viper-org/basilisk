// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_PARSER_AST_STATEMENT_VARIABLE_DECLARATION_H
#define BASILISK_FRAMEWORK_PARSER_AST_STATEMENT_VARIABLE_DECLARATION_H 1

#include "parser/ast/ASTNode.h"

#include <memory>

namespace parser
{
    class VariableDeclaration : public ASTNode
    {
    public:
        VariableDeclaration(Scope* scope, std::string name, Type* type, ASTNodePtr initValue, SourcePair source);

        virtual vipir::Value* codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag) override;

        virtual std::vector<ASTNode*> getChildren() override;

        virtual void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

    private:
        std::string mName;
        ASTNodePtr mInitValue;

        Symbol* mSymbol;
    };
    using VariableDeclarationPtr = std::unique_ptr<VariableDeclaration>;
}

#endif // BASILISK_FRAMEWORK_PARSER_AST_STATEMENT_VARIABLE_DECLARATION_H