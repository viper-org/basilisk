// Copyright 2025 solar-mist

#include "parser/ast/expression/MemberAccess.h"

#include "type/StructType.h"
#include "type/PointerType.h"
#include "type/PendingType.h"

#include <vipir/IR/Instruction/GEPInst.h>
#include <vipir/IR/Instruction/LoadInst.h>
#include <vipir/IR/Instruction/PtrCastInst.h>

#include <vipir/Type/PointerType.h>

#include <vipir/Module.h>

namespace parser
{
    MemberAccess::MemberAccess(Scope* scope, ASTNodePtr struc, std::string id, bool pointer, SourcePair source)
        : ASTNode(scope, std::move(source))
        , mStruct(std::move(struc))
        , mId(id)
        , mPointer(pointer)
    {
    }

    vipir::Value* MemberAccess::codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        vipir::Value* struc;
        if (mPointer)
        {
            struc = mStruct->dcodegen(builder, diBuilder, module, diag);
        }
        else
        {
            vipir::Value* structValue = mStruct->dcodegen(builder, diBuilder, module, diag);
            struc = vipir::getPointerOperand(structValue);

            vipir::Instruction* instruction = static_cast<vipir::Instruction*>(structValue);
            instruction->eraseFromParent();
        }

        vipir::Value* gep = builder.CreateStructGEP(struc, mStructType->getFieldOffset(mId));

        // struct types with a pointer to themselves cannot be emitted normally
        if (mStructType->getField(mId)->type->isPointerType())
        {
            if (static_cast<PointerType*>(mStructType->getField(mId)->type)->getPointeeType() == mStructType)
            {
                vipir::Type* type = vipir::PointerType::GetPointerType(vipir::PointerType::GetPointerType(mStructType->getVipirType()));
                gep = builder.CreatePtrCast(gep, type);
            }
        }

        return builder.CreateLoad(gep);
    }

    std::vector<ASTNode*> MemberAccess::getChildren()
    {
        return {mStruct.get()};
    }

    void MemberAccess::typeCheck(diagnostic::Diagnostics& diag, bool& exit)
    {
        mStruct->typeCheck(diag, exit);

        if (mPointer)
        {
            if (!mStruct->getType()->isPointerType())
            {
                diag.reportCompilerError(
                    mSource.start,
                    mSource.end,
                    std::format("{}'operator->'{} used on non-pointer value",
                        fmt::bold, fmt::defaults)
                );
                // TODO: Add note suggesting use of non-pointer member access operator
                exit = true;
                mType = Type::Get("error-type");
                return;
            }
            auto pointeeType = static_cast<PointerType*>(mStruct->getType())->getPointeeType();
            if (!pointeeType->isStructType())
            {
                diag.reportCompilerError(
                    mSource.start,
                    mSource.end,
                    std::format("{}'operator->'{} used on non-pointer-to-struct value with type '{}{}{}'",
                        fmt::bold, fmt::defaults, fmt::bold, pointeeType->getName(), fmt::defaults)
                );
                exit = true;
                mType = Type::Get("error-type");
                return;
            }
            if (auto pending = dynamic_cast<PendingType*>(pointeeType))
            {
                mStructType = pending->get();
            }
            else
            {
                mStructType = static_cast<StructType*>(pointeeType);
            }
        }
        else
        {
            if (!mStruct->getType()->isStructType())
            {
                diag.reportCompilerError(
                    mSource.start,
                    mSource.end,
                    std::format("{}'operator.'{} used on non-struct value",
                        fmt::bold, fmt::defaults)
                );
                exit = true;
                mType = Type::Get("error-type");
                return;
            }
            if (auto pending = dynamic_cast<PendingType*>(mStruct->getType()))
            {
                mStructType = pending->get();
            }
            else
            {
                mStructType = static_cast<StructType*>(mStruct->getType());
            }
        }

        auto structField = mStructType->getField(mId);
        if (structField)
            mType = structField->type;
        else
        {
            diag.reportCompilerError(
                mSource.start,
                mSource.end,
                std::format("struct '{}{}{}' has no member named '{}{}{}'",
                    fmt::bold, mStructType->getName(), fmt::defaults, fmt::bold, mId, fmt::defaults)
            );
            exit = true;
            mType = Type::Get("error-type");
        }
    }
}