// Copyright 2024 solar-mist

#include "parser/ast/expression/CastExpression.h"

#include <vipir/IR/Instruction/TruncInst.h>
#include <vipir/IR/Instruction/SExtInst.h>
#include <vipir/IR/Instruction/BinaryInst.h>
#include <vipir/IR/Constant/ConstantInt.h>

#include <cmath>

namespace parser
{
    CastExpression::CastExpression(Scope* scope, ASTNodePtr value, Type* destType)
        : ASTNode(scope, {}, destType)
        , mValue(std::move(value))
    {
    }

    vipir::Value* CastExpression::codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        auto value = mValue->dcodegen(builder, diBuilder, module, diag);
        if (mType->isIntegerType() && mValue->getType()->isIntegerType())
        {
            if (mType->getSize() < mValue->getType()->getSize())
            {
                return builder.CreateTrunc(value, mType->getVipirType());
            }
            else
            {
                return builder.CreateSExt(value, mType->getVipirType());
            }
        }
        else if (mType->isBooleanType() && mValue->getType()->isIntegerType())
        {
            auto constantInt = vipir::ConstantInt::Get(module, 0, mValue->getType()->getVipirType());
            return builder.CreateCmpNE(value, constantInt);
        }
        else if (mType->isIntegerType() && mValue->getType()->isBooleanType())
        {
            return builder.CreateSExt(value, mType->getVipirType());
        }
        return nullptr; // Should be unreachable
    }
    
    void CastExpression::typeCheck(diagnostic::Diagnostics& diag, bool& exit)
    {
        mValue->typeCheck(diag, exit);
    }

    bool CastExpression::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType)
    {
        return mValue->triviallyImplicitCast(diag, destType);
    }
}