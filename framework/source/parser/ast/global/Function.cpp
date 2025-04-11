// Copyright 2025 solar-mist

#include "parser/ast/global/Function.h"

#include <vipir/Type/FunctionType.h>
#include <vipir/IR/Function.h>

namespace parser
{
    Function::Function(std::string name, ScopePtr ownScope, std::vector<ASTNodePtr> body)
        : ASTNode(ownScope->parent)
        , mName(std::move(name))
        , mBody(std::move(body))
        , mOwnScope(std::move(ownScope))
    {
    }

    vipir::Value* Function::codegen(vipir::IRBuilder& builder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        vipir::FunctionType* functionType = vipir::FunctionType::Create(vipir::Type::GetIntegerType(32), {});

        vipir::Function* function = vipir::Function::Create(functionType, module, mName, false);

        vipir::BasicBlock* entryBB = vipir::BasicBlock::Create("", function);
        builder.setInsertPoint(entryBB);

        for (auto& node : mBody)
        {
            node->codegen(builder, module, diag);
        }

        return function;
    }
}