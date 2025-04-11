// Copyright 2025 solar-mist

#include "parser/ast/statement/ReturnStatement.h"

#include <vipir/IR/Instruction/RetInst.h>

#include <cmath>

namespace parser
{
    ReturnStatement::ReturnStatement(ASTNodePtr value)
        : mReturnValue(std::move(value))
    {
    }

    vipir::Value* ReturnStatement::codegen(vipir::IRBuilder& builder, vipir::Module& module)
    {
        auto returnValue = mReturnValue->codegen(builder, module);

        return builder.CreateRet(returnValue);
    }
}