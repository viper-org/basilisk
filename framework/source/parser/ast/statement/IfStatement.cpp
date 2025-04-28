// Copyright 2025 solar-mist

#include "parser/ast/statement/IfStatement.h"
#include "parser/ast/statement/VariableDeclaration.h"

#include <vipir/IR/BasicBlock.h>
#include <vipir/IR/Instruction/PhiInst.h>

namespace parser
{
    IfStatement::IfStatement(ASTNodePtr condition, ASTNodePtr body, ASTNodePtr elseBody, Scope* scope, SourcePair source)
        : ASTNode(scope, source)
        , mCondition(std::move(condition))
        , mBody(std::move(body))
        , mElseBody(std::move(elseBody))
    {
    }

    vipir::Value* IfStatement::codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        vipir::Value* condition = mCondition->dcodegen(builder, diBuilder, module, diag);

        vipir::BasicBlock* startBasicBlock = builder.getInsertPoint();

        vipir::BasicBlock* falseBasicBlock;
        if (mElseBody)
        {
            falseBasicBlock = vipir::BasicBlock::Create("", builder.getInsertPoint()->getParent());
        }
        vipir::BasicBlock* trueBasicBlock = vipir::BasicBlock::Create("", builder.getInsertPoint()->getParent());

        vipir::BasicBlock* mergeBasicBlock = vipir::BasicBlock::Create("", builder.getInsertPoint()->getParent());

        trueBasicBlock->loopEnd() = mergeBasicBlock;
        if (mElseBody)
        {
            falseBasicBlock->loopEnd() = mergeBasicBlock;
            builder.CreateCondBr(condition, trueBasicBlock, falseBasicBlock);
        }
        else
        {
            builder.CreateCondBr(condition, trueBasicBlock, mergeBasicBlock);
        }

        builder.setInsertPoint(trueBasicBlock);
        mBody->dcodegen(builder, diBuilder, module, diag);
        if (!builder.getInsertPoint()->hasTerminator())
            builder.CreateBr(mergeBasicBlock);

        if (mElseBody)
        {
            builder.setInsertPoint(falseBasicBlock);
            mElseBody->dcodegen(builder, diBuilder, module, diag);
            if (!builder.getInsertPoint()->hasTerminator())
                builder.CreateBr(mergeBasicBlock);
        }

        builder.setInsertPoint(mergeBasicBlock);

        // Build a list of all symbols that could have been modified
        std::vector<Symbol*> symbols;
        Scope* current = mScope;
        while (current)
        {
            for (auto& symbol : current->symbols)
            {
                symbols.push_back(symbol.get());
            }
            current = current->parent;
        }
        if (!mElseBody)
        {
            for (auto symbol : symbols)
            {
                auto trueBasicBlockValue = symbol->getLatestValue(trueBasicBlock);
                auto startBasicBlockValue = symbol->getLatestValue(startBasicBlock);
                if (trueBasicBlockValue && trueBasicBlockValue != startBasicBlockValue)
                {
                    auto phi = builder.CreatePhi(symbol->type->getVipirType());
                    phi->addIncoming(trueBasicBlockValue->value, trueBasicBlock);
                    phi->addIncoming(startBasicBlockValue->value, startBasicBlock);

                    auto q2 = builder.CreateQueryAddress();
                    symbol->getLatestValue()->end = q2;
                    symbol->values.push_back({mergeBasicBlock, phi, q2, nullptr});
                }
            }
        }
        else
        {
            for (auto symbol : symbols)
            {
                auto trueBasicBlockValue = symbol->getLatestValue(trueBasicBlock);
                auto falseBasicBlockValue = symbol->getLatestValue(falseBasicBlock);
                if (trueBasicBlockValue != falseBasicBlockValue)
                {
                    if (trueBasicBlockValue == nullptr)
                    {
                        trueBasicBlockValue = symbol->getLatestValue(startBasicBlock);
                    }
                    else if (falseBasicBlockValue == nullptr)
                    {
                        falseBasicBlockValue = symbol->getLatestValue(startBasicBlock);
                    }

                    auto phi = builder.CreatePhi(symbol->type->getVipirType());
                    phi->addIncoming(trueBasicBlockValue->value, trueBasicBlock);
                    phi->addIncoming(falseBasicBlockValue->value, falseBasicBlock);

                    auto q2 = builder.CreateQueryAddress();
                    symbol->getLatestValue()->end = q2;
                    symbol->values.push_back({mergeBasicBlock, phi, q2, nullptr});
                }
            }
        }

        return nullptr;
    }

    void IfStatement::typeCheck(diagnostic::Diagnostics& diag, bool& exit)
    {
        mCondition->typeCheck(diag, exit);
        mBody->typeCheck(diag, exit);
        if (mElseBody)
        {
            mElseBody->typeCheck(diag, exit);
        }

        if (dynamic_cast<VariableDeclaration*>(mBody.get()) || dynamic_cast<VariableDeclaration*>(mElseBody.get()))
        {
            diag.reportCompilerError(
                    mBody->getSourcePair().start,
                    mBody->getSourcePair().end,
                    std::format("variable declaration may not be used as body of if-statement")
            );
            exit = true;
        }

        if (!mCondition->getType()->isBooleanType())
        {
            auto boolType = Type::Get("bool");

            if (mCondition->canImplicitCast(diag, boolType))
            {
                mCondition = Cast(mCondition, boolType);
            }
            else
            {
                diag.reportCompilerError(
                    mCondition->getSourcePair().start,
                    mCondition->getSourcePair().end,
                    std::format("value of type '{}{}{}' cannot be used as a condition in if-statement",
                        fmt::bold, mCondition->getType()->getName(), fmt::defaults)
                );
                exit = true;
            }
        }
    }
}