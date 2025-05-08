// Copyright 2025 solar-mist

#include "parser/ast/expression/LenExpression.h"

#include "type/ArrayType.h"

#include <vipir/Module.h>

#include <vipir/IR/Function.h>
#include <vipir/IR/Constant/ConstantInt.h>

#include <vipir/IR/Instruction/GEPInst.h>
#include <vipir/IR/Instruction/LoadInst.h>

#include <cmath>

namespace parser
{
	LenExpression::LenExpression(Scope* scope, ASTNodePtr operand, SourcePair source)
		: ASTNode(scope, std::move(source))
        , mOperand(std::move(operand))
	{
	}

    vipir::Value* LenExpression::codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        if (mOperand->getType()->isSliceType())
        {
            auto slice = mOperand->codegen(builder, diBuilder, module, diag);
            if (auto load = dynamic_cast<vipir::LoadInst*>(slice))
            {
                auto pointerOperand = vipir::getPointerOperand(load);
                auto instruction = static_cast<vipir::Instruction*>(slice);
                instruction->eraseFromParent();

                auto lengthGep = builder.CreateStructGEP(pointerOperand, 1);
                return builder.CreateLoad(lengthGep);
            }
            else if (auto arg = dynamic_cast<vipir::Argument*>(slice))
            {
                auto lengthGep = builder.CreateStructGEP(arg, 1);
                return builder.CreateLoad(lengthGep);
            }
            return nullptr; // Should be unreachable
        }
        else if (mOperand->getType()->isArrayType())
        {
            auto arrayType = static_cast<ArrayType*>(mOperand->getType());
            return vipir::ConstantInt::Get(module, arrayType->getLength(), mType->getVipirType());
        }
        // Length of all other objects is 1
        return vipir::ConstantInt::Get(module, 1, mType->getVipirType());
    }

    std::vector<ASTNode*> LenExpression::getChildren()
    {
        return {mOperand.get()};
    }

    void LenExpression::typeCheck(diagnostic::Diagnostics& diag, bool& exit)
    {
        mOperand->typeCheck(diag, exit);

        mType = Type::Get("u64");
    }

    bool LenExpression::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType)
    {
        if (destType->isIntegerType())
        {
            if (mOperand->getType()->isSliceType())
            {
                return false;
            }
            mType = destType;
            return true;
        }
        return false;
    }
}