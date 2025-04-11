// Copyright 2025 solar-mist

#include "parser/ast/expression/IntegerLiteral.h"

#include <vipir/IR/Constant/ConstantInt.h>

#include <cmath>

namespace parser
{
    IntegerLiteral::IntegerLiteral(Scope* scope, std::uintmax_t value)
        : ASTNode(scope)
        , mValue(value)
    {
    }

    vipir::Value* IntegerLiteral::codegen(vipir::IRBuilder& builder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        return vipir::ConstantInt::Get(module, mValue, vipir::Type::GetIntegerType(32));
    }
}