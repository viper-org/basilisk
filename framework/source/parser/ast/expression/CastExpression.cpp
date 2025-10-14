// Copyright 2024 solar-mist

#include "parser/ast/expression/CastExpression.h"

#include "type/IntegerType.h"
#include "type/SliceType.h"
#include "type/PointerType.h"
#include "type/PendingType.h"

#include <vipir/Module.h>

#include <vipir/IR/Instruction/TruncInst.h>
#include <vipir/IR/Instruction/SExtInst.h>
#include <vipir/IR/Instruction/ZExtInst.h>
#include <vipir/IR/Instruction/BinaryInst.h>
#include <vipir/IR/Instruction/GEPInst.h>
#include <vipir/IR/Instruction/LoadInst.h>
#include <vipir/IR/Instruction/PtrCastInst.h>
#include <vipir/IR/Instruction/PtrToIntInst.h>
#include <vipir/IR/Instruction/IntToPtrInst.h>

#include <vipir/IR/Constant/ConstantInt.h>
#include <vipir/IR/Constant/ConstantStruct.h>

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
        if (auto pendingA = dynamic_cast<PendingType*>(mType))
        {
            if (pendingA->impl() == mValue->getType())
            {
                return value;
            }
        }
        if (auto pendingB = dynamic_cast<PendingType*>(mValue->getType()))
        {
            if (pendingB->impl() == mType)
            {
                return value;
            }
        }

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
        else if (mType->isPointerType() && mValue->getType()->isPointerType())
        {
            return builder.CreatePtrCast(value, mType->getVipirType());
        }
        else if (mType->isIntegerType() && mValue->getType()->isPointerType())
        {
            return builder.CreatePtrToInt(value, mType->getVipirType());
        }
        else if (mType->isPointerType() && mValue->getType()->isIntegerType())
        {
            if (mValue->getType()->getSize() < mType->getSize())
            {
                if (static_cast<IntegerType*>(mValue->getType())->isSigned())
                {
                    value = builder.CreateSExt(value, Type::Get("i64")->getVipirType());
                }
                else
                {
                    value = builder.CreateZExt(value, Type::Get("u64")->getVipirType());
                }
            }
            return builder.CreateIntToPtr(value, mType->getVipirType());
        }
        else if (mType->isSliceType() && mValue->getType()->isSliceType())
        {
            auto pointer = vipir::getPointerOperand(value);
            if (pointer)
            {
                auto instruction = static_cast<vipir::Instruction*>(value);
                instruction->eraseFromParent();
                value = pointer;
            }
            
            auto ptrGep = builder.CreateStructGEP(value, 0);
            auto ptr = builder.CreateLoad(ptrGep);
            auto lengthGep = builder.CreateStructGEP(value, 1);
            auto length = builder.CreateLoad(lengthGep);


            auto from = static_cast<SliceType*>(mValue->getType());
            auto to = static_cast<SliceType*>(mType);

            auto newPtr = builder.CreatePtrCast(ptr, PointerType::Get(to->getPointeeType())->getVipirType());
            if (from->getPointeeType()->isVoidType()) // void[] -> T[]
            {
                auto size = to->getPointeeType()->getSize() / 8;
                auto constant = vipir::ConstantInt::Get(module, size, vipir::Type::GetIntegerType(64));
                auto newLength = builder.CreateSDiv(length, constant);
                return builder.CreateConstantStruct(to->getVipirType(), {newPtr, newLength});
            }
            else // T[] -> void[]
            {
                auto size = from->getPointeeType()->getSize() / 8;
                auto constant = vipir::ConstantInt::Get(module, size, vipir::Type::GetIntegerType(64));
                auto newLength = builder.CreateSMul(length, constant);
                return builder.CreateConstantStruct(to->getVipirType(), {newPtr, newLength});
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

        if (mValue->getType()->castTo(mType) == Type::CastLevel::Disallowed)
        {
            diag.reportCompilerError(
                mSource.start,
                mSource.end,
                std::format("no cast from '{}{}{}' to '{}{}{}'",
                    fmt::bold, mValue->getType()->getName(), fmt::defaults,
                    fmt::bold, mType->getName(), fmt::defaults)
            );
            exit = true;
        }
    }

    bool CastExpression::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType)
    {
        return mValue->triviallyImplicitCast(diag, destType);
    }
}