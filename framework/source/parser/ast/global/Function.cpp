// Copyright 2025 solar-mist

#include "parser/ast/global/Function.h"

#include <vipir/Type/FunctionType.h>
#include <vipir/IR/Function.h>

namespace parser
{
    Function::Function(std::string name, std::vector<ASTNodePtr> body)
        : mName(std::move(name))
        , mBody(std::move(body))
    {
    }

    vipir::Value* Function::codegen(vipir::IRBuilder& builder, vipir::Module& module)
    {
        vipir::FunctionType* functionType = vipir::FunctionType::Create(vipir::Type::GetIntegerType(32), {});

        vipir::Function* function = vipir::Function::Create(functionType, module, mName, false);

        vipir::BasicBlock* entryBB = vipir::BasicBlock::Create("", function);
        builder.setInsertPoint(entryBB);

        for (auto& node : mBody)
        {
            node->codegen(builder, module);
        }

        return function;
    }
}