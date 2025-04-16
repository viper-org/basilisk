// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_PARSER_AST_GLOBAL_STRUCT_DECLARATION_H
#define BASILISK_FRAMEWORK_PARSER_AST_GLOBAL_STRUCT_DECLARATION_H 1

#include "parser/ast/ASTNode.h"
#include "parser/ast/global/Function.h"

#include <memory>
#include <string>
#include <vector>

namespace parser
{
    struct StructField
    {
        StructField(Type* type, std::string name);
        Type* type;
        std::string name;
    };

    class StructDeclaration : public ASTNode
    {
    public:
        StructDeclaration(Scope* scope, bool exported, bool pending, std::string name, std::vector<StructField> fields, SourcePair source);

        virtual vipir::Value* codegen(vipir::IRBuilder& builder, vipir::Module& module, diagnostic::Diagnostics& diag) override;

        virtual void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

    private:
        std::string mName;
        std::vector<StructField> mFields;

        Symbol* mSymbol;
    };
    using StructDeclarationPtr = std::unique_ptr<StructDeclaration>;
}

#endif // BASILISK_FRAMEWORK_PARSER_AST_GLOBAL_STRUCT_DECLARATION_H