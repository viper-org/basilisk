// Copyright 2025 solar-mist

#include "parser/ast/expression/SliceExpression.h"

#include "type/ArrayType.h"
#include "type/SliceType.h"
#include "vipir/Module.h"

#include <vipir/IR/Instruction/GEPInst.h>
#include <vipir/IR/Instruction/BinaryInst.h>
#include <vipir/IR/Instruction/LoadInst.h>

#include <vipir/IR/Constant/ConstantStruct.h>
#include <vipir/IR/Constant/ConstantInt.h>

namespace parser
{
    SliceExpression::SliceExpression(Scope* scope, ASTNodePtr slicee, ASTNodePtr start, ASTNodePtr end, SourcePair source)
        : ASTNode(scope, source)
        , mSlicee(std::move(slicee))
        , mStart(std::move(start))
        , mEnd(std::move(end))
    {
    }

    std::vector<ASTNode*> SliceExpression::getChildren()
    {
        return {mSlicee.get(), mStart.get(), mEnd.get()};
    }

    vipir::Value* SliceExpression::codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        auto slicee = mSlicee->codegen(builder, diBuilder, module, diag);
        auto start = mStart->codegen(builder, diBuilder, module, diag);
        auto end = mEnd->codegen(builder, diBuilder, module, diag);

        if (mSlicee->getType()->isSliceType())
        {
            auto pointer = vipir::getPointerOperand(slicee);
            auto instruction = static_cast<vipir::Instruction*>(slicee);
            instruction->eraseFromParent();
            
            auto ptrGep = builder.CreateStructGEP(pointer, 0);
            auto ptr = builder.CreateLoad(ptrGep);

            vipir::Value* newPtr = builder.CreateGEP(ptr, start);
            vipir::Value* newLength = builder.CreateSub(end, start);
            return builder.CreateConstantStruct(mType->getVipirType(), {newPtr, newLength});
        }
        return nullptr; // TODO: Array slicing
    }

    void SliceExpression::typeCheck(diagnostic::Diagnostics& diag, bool& exit)
    {
        mSlicee->typeCheck(diag, exit);
        mStart->typeCheck(diag, exit);
        mEnd->typeCheck(diag, exit);

        if (mSlicee->getType()->isSliceType())
        {
            mType = mSlicee->getType();
        }
        else if (mSlicee->getType()->isArrayType())
        {
            auto arrayType = static_cast<ArrayType*>(mSlicee->getType());
            mType = SliceType::Get(arrayType->getElementType());
        }
        else
        {
            diag.reportCompilerError(
                mSource.start,
                mSource.end,
                std::format("No match for slice with type '{}{}{}'",
                    fmt::bold, mSlicee->getType()->getName(), fmt::defaults)
            );
            exit = true;
            mType = Type::Get("error-type");
        }
    }
}