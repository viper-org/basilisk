// Copyright 2025 solar-mist

#include "parser/ast/global/Function.h"

#include <vipir/Type/FunctionType.h>
#include <vipir/IR/Function.h>

namespace parser
{
    Function::Function(std::string name, FunctionType* functionType, ScopePtr ownScope, std::vector<ASTNodePtr> body, SourcePair source)
        : ASTNode(ownScope->parent, source, functionType)
        , mName(std::move(name))
        , mBody(std::move(body))
        , mOwnScope(std::move(ownScope))
    {
        mOwnScope->currentReturnType = functionType->getReturnType();
    }

    vipir::Value* Function::codegen(vipir::IRBuilder& builder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        vipir::FunctionType* functionType = dynamic_cast<vipir::FunctionType*>(mType->getVipirType());

        // This should never happen but good to check just in case
        if (!functionType)
        {
            diag.fatalError("mType of parser::Function is not a function type.");
            std::exit(1);
        }

        vipir::Function* function = vipir::Function::Create(functionType, module, mName, false);

        vipir::BasicBlock* entryBB = vipir::BasicBlock::Create("", function);
        builder.setInsertPoint(entryBB);

        for (auto& node : mBody)
        {
            node->codegen(builder, module, diag);
        }

        return function;
    }

    void Function::typeCheck(diagnostic::Diagnostics& diag, bool& exit)
    {
        for (auto& node : mBody)
        {
            node->typeCheck(diag, exit);
        }
    }
}