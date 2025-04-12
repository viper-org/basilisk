// Copyright 2025 solar-mist

#include "parser/ast/expression/UnaryExpression.h"
#include "parser/ast/expression/VariableExpression.h"

#include "type/PointerType.h"

#include <vipir/Module.h>

#include <vipir/IR/Function.h>
#include <vipir/IR/Instruction/UnaryInst.h>
#include <vipir/IR/Instruction/LoadInst.h>
#include <vipir/IR/Instruction/GEPInst.h>
#include <vipir/IR/Instruction/AddrInst.h>
#include <vipir/IR/Instruction/AllocaInst.h>

namespace parser
{
	UnaryExpression::UnaryExpression(Scope* scope, ASTNodePtr operand, lexer::Token operatorToken, SourcePair source)
		: ASTNode(scope, std::move(source))
        , mOperand(std::move(operand))
        , mOperatorToken(std::move(operatorToken))
	{
		switch (mOperatorToken.getTokenType())
		{
            case lexer::TokenType::Star:
                mOperator = Operator::Indirection;
                break;

            case lexer::TokenType::Ampersand:
                mOperator = Operator::AddressOf;
                break;
            
            default:
                break; // Unreachable
        }
	}

    vipir::Value* UnaryExpression::codegen(vipir::IRBuilder& builder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        vipir::Value* operand = mOperand->codegen(builder, module, diag);

        switch (mOperator) 
        {
            case parser::UnaryExpression::Operator::Indirection:
                return builder.CreateLoad(operand);

            case parser::UnaryExpression::Operator::AddressOf:
            {
                if (auto var = dynamic_cast<VariableExpression*>(mOperand.get()))
                {
                    auto symbol = mScope->resolveSymbol(var->getName());

                    builder.insertAfter(operand);
                    auto alloca = builder.CreateAlloca(symbol->type->getVipirType());
                    builder.insertAfter(alloca);
                    builder.CreateStore(alloca, operand);
                    builder.insertAfter(nullptr);

                    symbol->values.push_back(std::make_pair(builder.getInsertPoint(), alloca));

                    return builder.CreateAddrOf(alloca);
                }

                auto pointerOperand = vipir::getPointerOperand(operand);
                auto instruction = static_cast<vipir::Instruction*>(operand);
                instruction->eraseFromParent();

                return builder.CreateAddrOf(pointerOperand);
            }

            default:
                break;
        }
        return nullptr; // Unreachable
    }

    void UnaryExpression::typeCheck(diagnostic::Diagnostics& diag, bool& exit)
    {
        mOperand->typeCheck(diag, exit);

        switch (mOperator) 
        {
            case Operator::Indirection:
                if (!mOperand->getType()->isPointerType())
                {
                    diag.reportCompilerError(
                        mSource.start,
                        mSource.end,
                        std::format("No match for '{}operator{}{} with type '{}{}{}'",
                            fmt::bold, mOperatorToken.getName(), fmt::defaults,
                            fmt::bold, mOperand->getType()->getName(), fmt::defaults)
                    );
                    exit = true;
                    mType = Type::Get("error-type");
                }
                else
                {
                    mType = static_cast<PointerType*>(mOperand->getType())->getPointeeType();
                }
                break;

            case Operator::AddressOf:
                mType = PointerType::Get(mOperand->getType());
                break;
        }
    }
}