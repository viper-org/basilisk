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
            case lexer::TokenType::Minus:
                mOperator = Operator::Negate;
                break;

            case lexer::TokenType::Tilde:
                mOperator = Operator::BWNot;
                break;

            case lexer::TokenType::Bang:
                mOperator = Operator::LogicalNot;
                break;

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

    vipir::Value* UnaryExpression::codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        vipir::Value* operand = mOperand->dcodegen(builder, diBuilder, module, diag);

        switch (mOperator) 
        {
            case Operator::Negate:
                return builder.CreateNeg(operand);

            case Operator::BWNot:
                return builder.CreateNot(operand);

            case Operator::Indirection:
                return builder.CreateLoad(operand);

            case Operator::AddressOf:
            {
                if (auto var = dynamic_cast<VariableExpression*>(mOperand.get()))
                {
                    auto symbol = mScope->resolveSymbol(var->getName());
                    if (operand)
                    {
                        if (auto alloca = dynamic_cast<vipir::AllocaInst*>(symbol->getLatestValue()->value))
                        {
                            return builder.CreateAddrOf(alloca, symbol->diVariable);
                        }
                    }

                    builder.insertAfter(operand);
                    auto alloca = builder.CreateAlloca(symbol->type->getVipirType());
                    builder.insertAfter(alloca);
                    if (operand)
                    {
                        builder.CreateStore(alloca, operand);
                    }
                    builder.insertAfter(nullptr);

                    auto q2 = builder.CreateQueryAddress();
                    if (operand) symbol->getLatestValue()->end = q2;
                    symbol->values.push_back({builder.getInsertPoint(), alloca, q2, nullptr });

                    return builder.CreateAddrOf(alloca, symbol->diVariable);
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

    vipir::Value* UnaryExpression::ccodegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag, vipir::BasicBlock* trueBB, vipir::BasicBlock* falseBB)
    {
        if (mOperator == Operator::LogicalNot)
        {
            mOperand->ccodegen(builder, diBuilder, module, diag, falseBB, trueBB);
        }

        return nullptr;
    }

    std::vector<ASTNode*> UnaryExpression::getChildren()
    {
        return {mOperand.get()};
    }

    void UnaryExpression::typeCheck(diagnostic::Diagnostics& diag, bool& exit)
    {
        mOperand->typeCheck(diag, exit);

        switch (mOperator) 
        {
            case Operator::Negate:
                if (!mOperand->getType()->isIntegerType())
                {
                    // implicit cast?
                    diag.reportCompilerError(
                        mSource.start,
                        mSource.end,
                        std::format("No match for '{}operator{}{}' with type '{}{}{}'",
                            fmt::bold, mOperatorToken.getName(), fmt::defaults,
                            fmt::bold, mOperand->getType()->getName(), fmt::defaults)
                    );
                    exit = true;
                    mType = Type::Get("error-type");
                }
                else
                {
                    mType = mOperand->getType();
                }
                break;

            case Operator::BWNot:
                if (!mOperand->getType()->isIntegerType())
                {
                    // implicit cast?
                    diag.reportCompilerError(
                        mSource.start,
                        mSource.end,
                        std::format("No match for '{}operator{}{}' with type '{}{}{}'",
                            fmt::bold, mOperatorToken.getName(), fmt::defaults,
                            fmt::bold, mOperand->getType()->getName(), fmt::defaults)
                    );
                    exit = true;
                    mType = Type::Get("error-type");
                }
                else
                {
                    mType = mOperand->getType();
                }
                break;

            case Operator::LogicalNot:
                if (!mOperand->getType()->isBooleanType())
                {
                    if (mOperand->canImplicitCast(diag, Type::Get("bool")))
                    {
                        mOperand = Cast(mOperand, Type::Get("bool"));
                    }
                    else
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
                }
                mType = Type::Get("bool");
                break;

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
