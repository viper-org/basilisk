// Copyright 2025 solar-mist

#include "parser/ast/statement/ReturnStatement.h"

#include <vipir/IR/Instruction/RetInst.h>

#include <cmath>

namespace parser
{
    ReturnStatement::ReturnStatement(Scope* scope, ASTNodePtr value)
        : ASTNode(scope)
        , mReturnValue(std::move(value))
    {
    }

    vipir::Value* ReturnStatement::codegen(vipir::IRBuilder& builder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        auto returnValue = mReturnValue->codegen(builder, module, diag);

        return builder.CreateRet(returnValue);
    }
}