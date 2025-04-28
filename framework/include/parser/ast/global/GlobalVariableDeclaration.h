// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_PARSER_AST_GLOBAL_GLOBAL_VARIABLE_DECLARATION_H
#define BASILISK_FRAMEWORK_PARSER_AST_GLOBAL_GLOBAL_VARIABLE_DECLARATION_H 1

#include "parser/ast/ASTNode.h"

#include <memory>

namespace parser
{
    class GlobalVariableDeclaration : public ASTNode
    {
    public:
        GlobalVariableDeclaration(Scope* scope, std::string name, Type* type, ASTNodePtr initValue, bool exported, SourcePair source);

        virtual vipir::Value* codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag) override;

        virtual void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

    private:
        std::string mName;
        ASTNodePtr mInitValue;

        Symbol* mSymbol;
    };
    using GlobalVariableDeclarationPtr = std::unique_ptr<GlobalVariableDeclaration>;
}

#endif // BASILISK_FRAMEWORK_PARSER_AST_GLOBAL_GLOBAL_VARIABLE_DECLARATION_H