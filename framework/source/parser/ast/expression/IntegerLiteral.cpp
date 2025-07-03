// Copyright 2025 solar-mist

#include "parser/ast/expression/IntegerLiteral.h"

#include <vipir/IR/Constant/ConstantInt.h>

#include <cmath>

namespace parser
{
    IntegerLiteral::IntegerLiteral(Scope* scope, std::uintmax_t value, Type* type, SourcePair source)
        : ASTNode(scope, source, type)
        , mValue(value)
    {
    }

    vipir::Value* IntegerLiteral::codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        return vipir::ConstantInt::Get(module, mValue, mType->getVipirType());
    }

    void IntegerLiteral::typeCheck(diagnostic::Diagnostics& diag, bool& exit)
    {
    }

    bool IntegerLiteral::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType)
    {
        if (destType->isIntegerType())
        {
            if (mValue >= std::pow(2, destType->getSize()))
            {
                diag.compilerWarning(
                    "implicit",
                    mSource.start,
                    mSource.end,
                    std::format("integer literal with value '{}{}{}' is being narrowed to '{}{}{}'",
                        fmt::bold, mValue, fmt::defaults,
                        fmt::bold, mValue % (std::uintmax_t)std::pow(2, destType->getSize()), fmt::defaults)
                );
            }

            mType = destType;
            return true;
        }
        return false;
    }

    ASTNodePtr IntegerLiteral::cloneExternal(Scope* in)
    {
        return std::make_unique<IntegerLiteral>(in, mValue, mType, mSource);
    }
}