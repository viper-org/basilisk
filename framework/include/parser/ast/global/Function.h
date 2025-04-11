// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_PARSER_AST_GLOBAL_FUNCTION_H
#define BASILISK_FRAMEWORK_PARSER_AST_GLOBAL_FUNCTION_H 1

#include "parser/ast/ASTNode.h"

#include "type/FunctionType.h"

#include <memory>
#include <string>
#include <vector>

namespace parser
{
    struct FunctionArgument
    {
        FunctionArgument(Type* type, std::string name);
        Type* type;
        std::string name;

        Symbol* symbol;
    };

    class Function : public ASTNode
    {
    public:
        Function(std::string name, FunctionType* functionType, std::vector<FunctionArgument> arguments, ScopePtr ownScope, std::vector<ASTNodePtr> body, SourcePair source);

        virtual vipir::Value* codegen(vipir::IRBuilder& builder, vipir::Module& module, diagnostic::Diagnostics& diag) override;

        virtual void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

    private:
        std::string mName;
        std::vector<FunctionArgument> mArguments;
        std::vector<ASTNodePtr> mBody;

        ScopePtr mOwnScope;
        Symbol* mSymbol;
    };
    using FunctionPtr = std::unique_ptr<Function>;
}

#endif // BASILISK_FRAMEWORK_PARSER_AST_GLOBAL_FUNCTION_H