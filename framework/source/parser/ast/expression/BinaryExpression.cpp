// Copyright 2025 solar-mist

#include "parser/ast/expression/BinaryExpression.h"
#include "parser/ast/expression/VariableExpression.h"

#include "type/IntegerType.h"

#include <vipir/Module.h>

#include <vipir/IR/Instruction/BinaryInst.h>
#include <vipir/IR/Instruction/StoreInst.h>
#include <vipir/IR/Instruction/AllocaInst.h>
#include <vipir/IR/Instruction/LoadInst.h>

namespace parser
{
	BinaryExpression::BinaryExpression(Scope* scope, ASTNodePtr left, lexer::Token operatorToken, ASTNodePtr right, SourcePair source)
		: ASTNode(scope, source)
		, mLeft(std::move(left))
		, mRight(std::move(right))
        , mOperatorToken(std::move(operatorToken))
	{
		switch (mOperatorToken.getTokenType())
		{
            case lexer::TokenType::Plus:
                mOperator = Operator::Add;
                break;
            case lexer::TokenType::Minus:
                mOperator = Operator::Sub;
                break;
            case lexer::TokenType::Star:
                mOperator = Operator::Mul;
                break;
            case lexer::TokenType::Slash:
                mOperator = Operator::Div;
                break;

            case lexer::TokenType::Equal:
                mOperator = Operator::Assign;
                break;
            default:
                break; // Unreachable
		}
	}

    vipir::Value* BinaryExpression::codegen(vipir::IRBuilder& builder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        vipir::Value* left = mLeft->codegen(builder, module, diag);
        vipir::Value* right = mRight->codegen(builder, module, diag);

        switch (mOperator) 
        {
            case Operator::Add:
                return builder.CreateAdd(left, right);
            case Operator::Sub:
                return builder.CreateSub(left, right);

            case Operator::Mul:
                if (mType->isIntegerType())
                {
                    if (static_cast<IntegerType*>(mType)->isSigned())
                        return builder.CreateSMul(left, right);
                    
                    return builder.CreateUMul(left, right);
                }

                diag.reportCompilerError(mSource.start, mSource.end, "Can't multiply a non-integer type");
                std::exit(1);
                break;

            case Operator::Div:
                if (mType->isIntegerType())
                {
                    if (static_cast<IntegerType*>(mType)->isSigned())
                        return builder.CreateSDiv(left, right);
                    
                    return builder.CreateUDiv(left, right);
                }

                diag.reportCompilerError(mSource.start, mSource.end, "Can't divide a non-integer type");
                std::exit(1);
                break;

            case Operator::Assign:
            {
                if (auto variableExpression = dynamic_cast<VariableExpression*>(mLeft.get()))
                {
                    auto symbol = mScope->resolveSymbol(variableExpression->getName());
                    if (dynamic_cast<vipir::AllocaInst*>(symbol->getLatestValue()))
                    {
                        auto instruction = static_cast<vipir::Instruction*>(left);
                        instruction->eraseFromParent();
                        
                        builder.CreateStore(symbol->getLatestValue(), right);
                    }
                    else
                    {
                        symbol->values.push_back(std::make_pair(builder.getInsertPoint(), right));
                    }
                    return nullptr;
                }
                else if (auto load = dynamic_cast<vipir::LoadInst*>(left))
                {
                    auto pointerOperand = vipir::getPointerOperand(load);
                    auto instruction = static_cast<vipir::Instruction*>(left);
                    instruction->eraseFromParent();

                    builder.CreateStore(pointerOperand, right);
                }
                // TODO: Implement
            }

            default:
                break;
        }
        return nullptr; // Unreachable
    }

    void BinaryExpression::typeCheck(diagnostic::Diagnostics& diag, bool& exit)
    {
        mLeft->typeCheck(diag, exit);
        mRight->typeCheck(diag, exit);

        switch (mOperator) 
        {
            case parser::BinaryExpression::Operator::Add:
            case parser::BinaryExpression::Operator::Sub:
            case parser::BinaryExpression::Operator::Mul:
            case parser::BinaryExpression::Operator::Div:
                if (mLeft->getType() != mRight->getType() && mLeft->getType()->isIntegerType() && mRight->getType()->isIntegerType())
                {
                    if (mLeft->getType()->getSize() > mRight->getType()->getSize()) // lhs > rhs
                    {
                        if (mRight->canImplicitCast(diag, mLeft->getType()))
                        {
                            mRight = Cast(mRight, mLeft->getType());
                        }
                        mType = mLeft->getType();
                    }
                    else // rhs > lhs
                    {
                        if (mLeft->canImplicitCast(diag, mRight->getType()))
                        {
                            mLeft = Cast(mLeft, mRight->getType());
                        }
                        mType = mRight->getType();
                    }
                }
                if (mLeft->getType() != mRight->getType() || !mLeft->getType()->isIntegerType())
                {
                    diag.reportCompilerError(
                        mSource.start,
                        mSource.end,
                        std::format("No match for '{}operator{}{} with types '{}{}{}' and '{}{}{}'",
                            fmt::bold, mOperatorToken.getName(), fmt::defaults,
                            fmt::bold, mLeft->getType()->getName(), fmt::defaults,
                            fmt::bold, mRight->getType()->getName(), fmt::defaults)
                    );
                    exit = true;
                }
                mType = mLeft->getType(); // Types are the same
                break;

            case Operator::Assign:
                if (mLeft->getType() != mRight->getType())
                {
                    if (mRight->canImplicitCast(diag, mLeft->getType()))
                    {
                        mRight = Cast(mRight, mLeft->getType());
                    }
                    else
                    {
                        diag.reportCompilerError(
                            mSource.start,
                            mSource.end,
                            std::format("No match for '{}operator{}{} with types '{}{}{}' and '{}{}{}'",
                                fmt::bold, mOperatorToken.getName(), fmt::defaults,
                                fmt::bold, mLeft->getType()->getName(), fmt::defaults,
                                fmt::bold, mRight->getType()->getName(), fmt::defaults)
                        );
                        exit = true;
                    }
                }
                break;

            default:
                break;
        }
    }
}