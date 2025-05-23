// Copyright 2025 solar-mist

#include "parser/ast/expression/BinaryExpression.h"
#include "parser/ast/expression/VariableExpression.h"

#include "type/IntegerType.h"
#include "type/ArrayType.h"
#include "type/PointerType.h"
#include "type/SliceType.h"
#include "vipir/IR/Argument.h"

#include <vipir/Module.h>

#include <vipir/IR/Instruction/BinaryInst.h>
#include <vipir/IR/Instruction/StoreInst.h>
#include <vipir/IR/Instruction/AllocaInst.h>
#include <vipir/IR/Instruction/LoadInst.h>
#include <vipir/IR/Instruction/GEPInst.h>
#include <vipir/IR/Instruction/AddrInst.h>

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

            case lexer::TokenType::DoubleEqual:
                mOperator = Operator::Equal;
                break;
            case lexer::TokenType::BangEqual:
                mOperator = Operator::NotEqual;
                break;
            case lexer::TokenType::LessThan:
                mOperator = Operator::LessThan;
                break;
            case lexer::TokenType::GreaterThan:
                mOperator = Operator::GreaterThan;
                break;
            case lexer::TokenType::LessEqual:
                mOperator = Operator::LessEqual;
                break;
            case lexer::TokenType::GreaterEqual:
                mOperator = Operator::GreaterEqual;
                break;

            case lexer::TokenType::Ampersand:
                mOperator = Operator::BWAnd;
                break;
            case lexer::TokenType::Pipe:
                mOperator = Operator::BWOr;
                break;
            case lexer::TokenType::Caret:
                mOperator = Operator::BWXor;
                break;

            case lexer::TokenType::DoubleAmpersand:
                mOperator = Operator::LogicalAnd;
                break;
            case lexer::TokenType::DoublePipe:
                mOperator = BinaryExpression::Operator::LogicalOr;
                break;

            case lexer::TokenType::Equal:
                mOperator = Operator::Assign;
                break;

            case lexer::TokenType::PlusEqual:
                mOperator = Operator::AddAssign;
                break;
            case lexer::TokenType::MinusEqual:
                mOperator = Operator::SubAssign;
                break;

            case lexer::TokenType::LeftBracket:
                mOperator = Operator::Index;
                break;

            default:
                break; // Unreachable
		}
	}

    vipir::Value* BinaryExpression::codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        vipir::Value* left = mLeft->dcodegen(builder, diBuilder, module, diag);
        vipir::Value* right = mRight->dcodegen(builder, diBuilder, module, diag);

        auto createAssign = [&](vipir::Value* left, vipir::Value* right) -> vipir::Value* {
            if (auto variableExpression = dynamic_cast<VariableExpression*>(mLeft.get()))
            {
                auto symbol = mScope->resolveSymbol(variableExpression->getName());
                if (symbol->constant) {
                    diag.reportCompilerError(
                        mSource.start,
                        mSource.end,
                        "attempted to mutate the value of a constant"
                    );
                    std::exit(1);
                }
              
                if (dynamic_cast<vipir::AllocaInst*>(symbol->getLatestValue()->value))
                {
                    auto instruction = static_cast<vipir::Instruction*>(left);
                    instruction->eraseFromParent();
                    
                    builder.CreateStore(symbol->getLatestValue()->value, right);

                    return right;
                }
                else if (dynamic_cast<vipir::GlobalVar*>(symbol->getLatestValue()->value))
                {
                    auto instruction = static_cast<vipir::Instruction*>(left);
                    instruction->eraseFromParent();
                    
                    builder.CreateStore(symbol->getLatestValue()->value, right);

                    return right;
                }
                else
                {
                    auto q2 = builder.CreateQueryAddress();
                    symbol->getLatestValue()->end = q2;
                    vipir::DIVariable* pointer = nullptr;
                    if (auto addr = dynamic_cast<vipir::AddrInst*>(right))
                    {
                        if (addr->getDebugVariable())
                        {
                            pointer = addr->getDebugVariable();
                        }
                    }
                    symbol->values.push_back({builder.getInsertPoint(), right, q2, nullptr, pointer});
                    return right;
                }
            }
            else if (auto load = dynamic_cast<vipir::LoadInst*>(left))
            {
                auto pointerOperand = vipir::getPointerOperand(load);
                auto instruction = static_cast<vipir::Instruction*>(left);
                instruction->eraseFromParent();

                builder.CreateStore(pointerOperand, right);

                return right;
            }
            return nullptr;
        };

        switch (mOperator)
        {
            case Operator::Add:
                if (mLeft->getType()->isPointerType())
                {
                    return builder.CreateGEP(left, right);
                }

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

            case Operator::Equal:
                return builder.CreateCmpEQ(left, right);
            case Operator::NotEqual:
                return builder.CreateCmpNE(left, right);
            case Operator::LessThan:
                return builder.CreateCmpLT(left, right);
            case Operator::GreaterThan:
                return builder.CreateCmpGT(left, right);
            case Operator::LessEqual:
                return builder.CreateCmpLE(left, right);
            case Operator::GreaterEqual:
                return builder.CreateCmpGE(left, right);

            case Operator::BWAnd:
                return builder.CreateBWAnd(left, right);
            case Operator::BWOr:
                return builder.CreateBWOr(left, right);
            case Operator::BWXor:
                return builder.CreateBWXor(left, right);

            case Operator::Assign:
                return createAssign(left, right);
            
            case Operator::AddAssign:
                if (mLeft->getType()->isPointerType())
                {
                    auto gep = builder.CreateGEP(left, right);
                    return createAssign(left, gep);
                }
                else
                {
                    auto add = builder.CreateAdd(left, right);
                    return createAssign(left, add);
                }
            
            case Operator::SubAssign:
            {
                auto sub = builder.CreateSub(left, right);
                return createAssign(left, sub);
            }

            case Operator::Index:
                if (mLeft->getType()->isPointerType())
                {
                    auto gep = builder.CreateGEP(left, right);
                    return builder.CreateLoad(gep);
                }
                else
                {
                    if (auto load = dynamic_cast<vipir::LoadInst*>(left))
                    {
                        auto pointerOperand = vipir::getPointerOperand(load);
                        auto instruction = static_cast<vipir::Instruction*>(left);
                        instruction->eraseFromParent();
                        
                        if (mLeft->getType()->isSliceType())
                        {
                            auto fieldGep = builder.CreateStructGEP(pointerOperand, 0);
                            auto field = builder.CreateLoad(fieldGep);
                            auto sliceGep = builder.CreateGEP(field, right);
                            return builder.CreateLoad(sliceGep);
                        }
                        auto gep = builder.CreateGEP(pointerOperand, right);
                        return builder.CreateLoad(gep);
                    }
                    else if (auto arg = dynamic_cast<vipir::Argument*>(left))
                    {
                        if (mLeft->getType()->isSliceType())
                        {
                            auto fieldGep = builder.CreateStructGEP(arg, 0);
                            auto field = builder.CreateLoad(fieldGep);
                            auto sliceGep = builder.CreateGEP(field, right);
                            return builder.CreateLoad(sliceGep);
                        }
                        auto gep = builder.CreateGEP(arg, right);
                        return builder.CreateLoad(gep);
                    }
                }
                break;

            default:
                break;
        }
        return nullptr; // Unreachable
    }

    vipir::Value* BinaryExpression::ccodegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag, vipir::BasicBlock* trueBB, vipir::BasicBlock* falseBB)
    {
        if (mOperator == Operator::LogicalAnd)
        {
            auto newBB = vipir::BasicBlock::Create("", trueBB->getParent());

            mLeft->ccodegen(builder, diBuilder, module, diag, newBB, falseBB);
            builder.setInsertPoint(newBB);
            mRight->ccodegen(builder, diBuilder, module, diag, trueBB, falseBB);
        }
        else if (mOperator == Operator::LogicalOr)
        {
            auto newBB = vipir::BasicBlock::Create("", trueBB->getParent());

            mLeft->ccodegen(builder, diBuilder, module, diag, trueBB, newBB);
            builder.setInsertPoint(newBB);
            mRight->ccodegen(builder, diBuilder, module, diag, trueBB, falseBB);
        }
        else
        {
            vipir::Value* condition;
            vipir::Value* left  = mLeft->dcodegen(builder, diBuilder, module, diag);
            vipir::Value* right = mRight->dcodegen(builder, diBuilder, module, diag);
            switch (mOperator)
            {
                case Operator::Equal:
                    condition = builder.CreateCmpEQ(left, right);
                    break;
                case Operator::NotEqual:
                    condition = builder.CreateCmpNE(left, right);
                    break;
                case Operator::LessThan:
                    condition = builder.CreateCmpLT(left, right);
                    break;
                case Operator::GreaterThan:
                    condition = builder.CreateCmpGT(left, right);
                    break;
                case Operator::LessEqual:
                    condition = builder.CreateCmpLE(left, right);
                    break;
                case Operator::GreaterEqual:
                    condition = builder.CreateCmpGE(left, right);
                    break;

                default:
                    return nullptr;
            }
            builder.CreateCondBr(condition, trueBB, falseBB);
        }

        return nullptr;
    }

    std::vector<ASTNode*> BinaryExpression::getChildren()
    {
        return {mLeft.get(), mRight.get()};
    }

    void BinaryExpression::typeCheck(diagnostic::Diagnostics& diag, bool& exit)
    {
        mLeft->typeCheck(diag, exit);
        mRight->typeCheck(diag, exit);

        switch (mOperator) 
        {
            case parser::BinaryExpression::Operator::Add:
                if (mLeft->getType()->isPointerType())
                {
                    if (!mRight->getType()->isIntegerType())
                    {
                        if (mRight->canImplicitCast(diag, Type::Get("i64")))
                        {
                            mRight = Cast(mRight, Type::Get("i64"));
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
                            mType = Type::Get("error-type");
                            break;
                        }
                    }
                    mType = mLeft->getType();
                    break;
                }
            case parser::BinaryExpression::Operator::Sub:
            case parser::BinaryExpression::Operator::Mul:
            case parser::BinaryExpression::Operator::Div:
            case parser::BinaryExpression::Operator::BWAnd: //
            case parser::BinaryExpression::Operator::BWOr:  // Not sure if these should be their own paths, but to me it seems like they need the exact same type checking
            case parser::BinaryExpression::Operator::BWXor: //
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

            case Operator::Equal:
            case Operator::NotEqual:
            case Operator::LessThan:
            case Operator::GreaterThan:
            case Operator::LessEqual:
            case Operator::GreaterEqual:
                if (mLeft->getType() != mRight->getType())
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
                if (mLeft->getType() != mRight->getType()) // unable to cast
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
                mType = Type::Get("bool");
                break;

            case parser::BinaryExpression::Operator::LogicalAnd:
            case parser::BinaryExpression::Operator::LogicalOr:
            {
                auto checkOperand = [this, &diag, &exit](ASTNodePtr& operand){
                    if (!operand->getType()->isBooleanType())
                    {
                        if (operand->canImplicitCast(diag, Type::Get("bool")))
                        {
                            operand = Cast(mLeft, Type::Get("bool"));
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
                };
                checkOperand(mLeft);
                checkOperand(mRight);
                mType = Type::Get("bool");
                break;
            }

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
                            std::format("No match for '{}operator{}{}' with types '{}{}{}' and '{}{}{}'",
                                fmt::bold, mOperatorToken.getName(), fmt::defaults,
                                fmt::bold, mLeft->getType()->getName(), fmt::defaults,
                                fmt::bold, mRight->getType()->getName(), fmt::defaults)
                        );
                        exit = true;
                    }
                }
                break;

            case parser::BinaryExpression::Operator::AddAssign:
            case parser::BinaryExpression::Operator::SubAssign:
                if (mLeft->getType()->isPointerType())
                {
                    if (!mRight->getType()->isIntegerType())
                    {
                        if (mRight->canImplicitCast(diag, Type::Get("i64")))
                        {
                            mRight = Cast(mRight, Type::Get("i64"));
                        }
                        else
                        {
                            diag.reportCompilerError(
                                mSource.start,
                                mSource.end,
                                std::format("No match for '{}operator{}{}' with types '{}{}{}' and '{}{}{}'",
                                    fmt::bold, mOperatorToken.getName(), fmt::defaults,
                                    fmt::bold, mLeft->getType()->getName(), fmt::defaults,
                                    fmt::bold, mRight->getType()->getName(), fmt::defaults)
                            );
                            exit = true;
                        }
                    }
                }
                else if (mLeft->getType()->isIntegerType())
                {
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
                                std::format("No match for '{}operator{}{}' with types '{}{}{}' and '{}{}{}'",
                                    fmt::bold, mOperatorToken.getName(), fmt::defaults,
                                    fmt::bold, mLeft->getType()->getName(), fmt::defaults,
                                    fmt::bold, mRight->getType()->getName(), fmt::defaults)
                            );
                            exit = true;
                        }
                        mType = mLeft->getType();
                    }
                }
                else
                {
                    diag.reportCompilerError(
                        mSource.start,
                        mSource.end,
                        std::format("No match for '{}operator{}{}' with types '{}{}{}' and '{}{}{}'",
                            fmt::bold, mOperatorToken.getName(), fmt::defaults,
                            fmt::bold, mLeft->getType()->getName(), fmt::defaults,
                            fmt::bold, mRight->getType()->getName(), fmt::defaults)
                    );
                    exit = true;
                }
                break;
            
            case Operator::Index:
                if (!mLeft->getType()->isArrayType() && !mLeft->getType()->isPointerType() && !mLeft->getType()->isSliceType())
                {
                    diag.reportCompilerError(
                        mSource.start,
                        mSource.end,
                        std::format("No match for '{}operator[]{}' with type '{}{}{}'",
                            fmt::bold, fmt::defaults,
                            fmt::bold, mLeft->getType()->getName(), fmt::defaults)
                    );
                    exit = true;
                }
                if (!mRight->getType()->isIntegerType())
                {
                    if (mRight->canImplicitCast(diag, Type::Get("i32")))
                    {
                        mRight = Cast(mRight, Type::Get("i32"));
                    }
                    else
                    {
                        diag.reportCompilerError(
                            mSource.start,
                            mSource.end,
                            std::format("No match for '{}operator[]{}' with index type '{}{}{}'",
                                fmt::bold, fmt::defaults,
                                fmt::bold, mRight->getType()->getName(), fmt::defaults)
                        );
                        exit = true;
                    }
                }
                if (mLeft->getType()->isArrayType())
                {
                    mType = static_cast<ArrayType*>(mLeft->getType())->getElementType();
                }
                else if (mLeft->getType()->isPointerType())
                {
                    mType = static_cast<PointerType*>(mLeft->getType())->getPointeeType();
                }
                else if (mLeft->getType()->isSliceType())
                {
                    mType = static_cast<SliceType*>(mLeft->getType())->getPointeeType();
                }
                break;

            default:
                break;
        }
    }
}