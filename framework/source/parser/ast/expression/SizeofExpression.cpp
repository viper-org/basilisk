// Copyright 2025 solar-mist

#include "parser/ast/expression/SizeofExpression.h"

#include <vipir/Module.h>

#include <vipir/IR/Function.h>
#include <vipir/IR/Constant/ConstantInt.h>

#include <cmath>

namespace parser
{
	SizeofExpression::SizeofExpression(Scope* scope, ASTNodePtr operand, SourcePair source)
		: ASTNode(scope, std::move(source))
        , mOperandExpression(std::move(operand))
	{
	}

	SizeofExpression::SizeofExpression(Scope* scope, Type* operand, SourcePair source)
		: ASTNode(scope, std::move(source))
        , mOperandType(std::move(operand))
	{
	}

    vipir::Value* SizeofExpression::codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        if (mOperandExpression)
            return vipir::ConstantInt::Get(module, mOperandExpression->getType()->getVipirType()->getSizeInBits() / 8, mType->getVipirType());
    
        return vipir::ConstantInt::Get(module, mOperandType->getVipirType()->getSizeInBits() / 8, mType->getVipirType());
    }

    std::vector<ASTNode*> SizeofExpression::getChildren()
    {
        if (mOperandExpression)
            return {mOperandExpression.get()};

        return {};
    }

    void SizeofExpression::typeCheck(diagnostic::Diagnostics& diag, bool& exit)
    {
        if (mOperandExpression)
            mOperandExpression->typeCheck(diag, exit);

        mType = Type::Get("u32");
    }

    bool SizeofExpression::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType)
    {
        int value;
        if (mOperandExpression)
            value = mOperandExpression->getType()->getVipirType()->getSizeInBits() / 8;
        else
            value = mOperandType->getVipirType()->getSizeInBits() / 8;

        if (destType->isIntegerType())
        {
            if (value >= std::pow(2, destType->getSize()))
            {
                diag.compilerWarning(
                    "implicit",
                    mSource.start,
                    mSource.end,
                    std::format("integer literal with value '{}{}{}' is being narrowed to '{}{}{}'",
                        fmt::bold, value, fmt::defaults,
                        fmt::bold, value % (std::uintmax_t)std::pow(2, destType->getSize()), fmt::defaults)
                );
            }

            mType = destType;
            return true;
        }
        return false;
    }
}