// Copyright 2024 solar-mist

#include "parser/ast/expression/CastExpression.h"

#include "type/IntegerType.h"

#include <vipir/IR/Instruction/TruncInst.h>
#include <vipir/IR/Instruction/SExtInst.h>
#include <vipir/IR/Instruction/ZExtInst.h>
#include <vipir/IR/Instruction/BinaryInst.h>
#include <vipir/IR/Constant/ConstantInt.h>

#include <cmath>

namespace parser
{
    CastExpression::CastExpression(Scope* scope, ASTNodePtr value, Type* destType, SourcePair source)
        : ASTNode(scope, std::move(source), destType)
        , mValue(std::move(value))
    {
    }

    vipir::Value* CastExpression::codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        auto value = mValue->dcodegen(builder, diBuilder, module, diag);
        if (mType->isIntegerType() && mValue->getType()->isIntegerType())
        {
            auto intA = static_cast<IntegerType*>(mType);
            auto intB = static_cast<IntegerType*>(mValue->getType());
            if (intA->isSigned() && intB->isSigned())
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
            else if (!intA->isSigned() && !intB->isSigned())
            {
                if (mType->getSize() < mValue->getType()->getSize())
                {
                    return builder.CreateTrunc(value, mType->getVipirType());
                }
                else
                {
                    return builder.CreateZExt(value, mType->getVipirType());
                }
            }
            else if (intA->isSigned() && !intB->isSigned())
            {
                if (mType->getSize() < mValue->getType()->getSize())
                {
                    return builder.CreateTrunc(value, mType->getVipirType());
                }
                else if (mType->getSize() > mValue->getType()->getSize())
                {
                    return builder.CreateSExt(value, mType->getVipirType());
                }
                else
                {
                    return value; // Implementation type is the same
                }
            }
            else if (!intA->isSigned() && intB->isSigned())
            {
                if (mType->getSize() < mValue->getType()->getSize())
                {
                    return builder.CreateTrunc(value, mType->getVipirType());
                }
                else if (mType->getSize() > mValue->getType()->getSize())
                {
                    return builder.CreateZExt(value, mType->getVipirType());
                }
                else
                {
                    return value; // Implementation type is the same
                }
            }
        }
        else if (mType->isBooleanType() && mValue->getType()->isIntegerType())
        {
            auto constantInt = vipir::ConstantInt::Get(module, 0, mValue->getType()->getVipirType());
            return builder.CreateCmpNE(value, constantInt);
        }
        else if (mType->isIntegerType() && mValue->getType()->isBooleanType())
        {
            auto intTy = static_cast<IntegerType*>(mType);
            if (intTy->isSigned())
            {
                return builder.CreateSExt(value, mType->getVipirType());
            }
            else
            {
                return builder.CreateZExt(value, mType->getVipirType());
            }
        }
        return nullptr; // Should be unreachable
    }

    std::vector<ASTNode*> CastExpression::getChildren()
    {
        return {mValue.get()};
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