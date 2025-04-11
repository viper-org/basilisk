// Copyright 2025 solar-mist

#include "parser/ast/statement/VariableDeclaration.h"

#include <vipir/IR/Instruction/AllocaInst.h>

#include <algorithm>

namespace parser
{
    VariableDeclaration::VariableDeclaration(Scope* scope, std::string name, ASTNodePtr initValue)
        : ASTNode(scope)
        , mName(std::move(name))
        , mInitValue(std::move(initValue))
    {
        mScope->symbols.push_back(std::make_unique<Symbol>(mName));
        mSymbol = mScope->symbols.back().get();
    }

    vipir::Value* VariableDeclaration::codegen(vipir::IRBuilder& builder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        if (mInitValue)
        {
            vipir::Value* initValue = mInitValue->codegen(builder, module, diag);
            mSymbol->values.push_back(std::make_pair(builder.getInsertPoint(), initValue));
        }

        return nullptr;
    }
}