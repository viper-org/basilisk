// Copyright 2025 solar-mist

#include "parser/ast/expression/BooleanLiteral.h"

#include <vipir/IR/Constant/ConstantBool.h>

#include <cmath>

namespace parser
{
    BooleanLiteral::BooleanLiteral(Scope* scope, bool value, SourcePair source)
        : ASTNode(scope, source, Type::Get("bool"))
        , mValue(value)
    {
    }

    vipir::Value* BooleanLiteral::codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        return builder.CreateConstantBool(mValue);
    }

    void BooleanLiteral::typeCheck(diagnostic::Diagnostics& diag, bool& exit)
    {
    }

    bool BooleanLiteral::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType)
    {
        if (destType->isIntegerType())
        {
            mType = destType;
            return true;
        }
        return false;
    }
}