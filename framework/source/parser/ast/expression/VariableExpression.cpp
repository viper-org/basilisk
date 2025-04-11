// Copyright 2025 solar-mist

#include "parser/ast/expression/VariableExpression.h"

#include <vipir/IR/Function.h>
#include <vipir/IR/Instruction/LoadInst.h>
#include <vipir/IR/Instruction/AllocaInst.h>
#include <vipir/IR/Instruction/GEPInst.h>

#include <cmath>

namespace parser
{
    VariableExpression::VariableExpression(Scope* scope, std::string name)
        : ASTNode(scope)
        , mName(std::move(name))
    {
    }

    vipir::Value* VariableExpression::codegen(vipir::IRBuilder& builder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        Symbol* symbol = mScope->resolveSymbol(mName);
        
        return symbol->getLatestValue(builder.getInsertPoint());
    }
}