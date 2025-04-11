// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_PARSER_AST_GLOBAL_FUNCTION_H
#define BASILISK_FRAMEWORK_PARSER_AST_GLOBAL_FUNCTION_H 1

#include "parser/ast/ASTNode.h"

#include <memory>
#include <string>
#include <vector>

namespace parser
{
    class Function : public ASTNode
    {
    public:
        Function(std::string name, std::vector<ASTNodePtr> body);

        virtual vipir::Value* codegen(vipir::IRBuilder& builder, vipir::Module& module) override;

    private:
        std::string mName;
        std::vector<ASTNodePtr> mBody;
    };
    using FunctionPtr = std::unique_ptr<Function>;
}

#endif // BASILISK_FRAMEWORK_PARSER_AST_GLOBAL_FUNCTION_H