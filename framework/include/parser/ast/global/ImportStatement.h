// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_PARSER_AST_GLOBAL_IMPORT_STATEMENT_H
#define BASILISK_FRAMEWORK_PARSER_AST_GLOBAL_IMPORT_STATEMENT_H 1

#include "parser/ast/ASTNode.h"

#include <memory>

namespace parser
{
    class ImportStatement : public ASTNode
    {
    public:
        ImportStatement(Scope* scope, std::vector<std::string> module, SourcePair source);

        virtual vipir::Value* codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag) override;

        virtual void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

        std::vector<std::string> getModule();

    private:
        std::vector<std::string> mModule;
    };
    using ImportStatementPtr = std::unique_ptr<ImportStatement>;
}

#endif // BASILISK_FRAMEWORK_PARSER_AST_GLOBAL_IMPORT_STATEMENT_H