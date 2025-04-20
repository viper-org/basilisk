// Copyright 2025 solar-mist

#include "parser/ast/statement/WhileStatement.h"
#include "parser/ast/statement/VariableDeclaration.h"

#include <vipir/IR/BasicBlock.h>
#include <vipir/IR/Function.h>
#include <vipir/IR/Instruction/PhiInst.h>
#include <vipir/IR/Instruction/AllocaInst.h>

#include <algorithm>

namespace parser
{
    WhileStatement::WhileStatement(ASTNodePtr condition, ASTNodePtr body, Scope* scope, SourcePair source)
        : ASTNode(scope, source)
        , mCondition(std::move(condition))
        , mBody(std::move(body))
    {
    }

    vipir::Value* WhileStatement::codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        vipir::BasicBlock* startBasicBlock = builder.getInsertPoint();

        //vipir::BasicBlock* conditionBasicBlock = vipir::BasicBlock::Create("", builder.getInsertPoint()->getParent());
        vipir::BasicBlock* bodyBasicBlock = vipir::BasicBlock::Create("", builder.getInsertPoint()->getParent());
        vipir::BasicBlock* mergeBasicBlock = vipir::BasicBlock::Create("", builder.getInsertPoint()->getParent());

        // Build a list of all symbols that could have been modified
        std::vector<Symbol*> symbols;
        std::vector<vipir::PhiInst*> phis; // Indices are the same as the above vector
        Scope* current = mScope;
        while (current)
        {
            for (auto& symbol : current->symbols)
            {
                symbols.push_back(symbol.get());
            }
            current = current->parent;
        }

        vipir::Value* precondition = mCondition->dcodegen(builder, diBuilder, module, diag);
        builder.CreateCondBr(precondition, bodyBasicBlock, mergeBasicBlock);

        bodyBasicBlock->loopEnd() = mergeBasicBlock;

        builder.setInsertPoint(bodyBasicBlock);
        for (auto symbol : symbols)
        {
            auto startBasicBlockValue = symbol->getLatestValue(startBasicBlock);
            if (!startBasicBlockValue || dynamic_cast<vipir::AllocaInst*>(startBasicBlockValue->value))
            {
                phis.push_back(nullptr);
                continue;
            }
            auto phi = builder.CreatePhi(symbol->type->getVipirType());
            phi->addIncoming(startBasicBlockValue->value, startBasicBlock);
            phis.push_back(phi);

            auto q2 = builder.CreateQueryAddress();
            startBasicBlockValue->end = q2;
            symbol->values.push_back({bodyBasicBlock, phi, q2, nullptr});
        }
        mBody->dcodegen(builder, diBuilder, module, diag);
        vipir::Value* condition = mCondition->dcodegen(builder, diBuilder, module, diag);
        builder.CreateCondBr(condition, bodyBasicBlock, mergeBasicBlock);
        
        for (int i = 0; i < phis.size(); ++i)
        {
            if (!phis[i]) continue;

            auto bodyBasicBlockValue = symbols[i]->getLatestValue();
            auto startBasicBlockValue = symbols[i]->getLatestValue(startBasicBlock);
            if (bodyBasicBlockValue && bodyBasicBlockValue != startBasicBlockValue && !dynamic_cast<vipir::PhiInst*>(bodyBasicBlockValue->value))
            {
                phis[i]->addIncoming(bodyBasicBlockValue->value, bodyBasicBlockValue->bb);
                auto q2 = builder.CreateQueryAddress();
                bodyBasicBlockValue->end = q2;
                symbols[i]->values.push_back({mergeBasicBlock, phis[i], q2, nullptr});
            }
            else
            {
                auto it = std::find_if(symbols[i]->values.begin(), symbols[i]->values.end(), [bodyBasicBlockValue](const auto& value) {
                    return value.value == bodyBasicBlockValue->value;
                });
                symbols[i]->values.erase(it);
                builder.getInsertPoint()->getParent()->replaceAllUsesWith(phis[i], symbols[i]->getLatestValue(startBasicBlock)->value);
                phis[i]->eraseFromParent();
            }
        }

        builder.setInsertPoint(mergeBasicBlock);
        return nullptr;
    }

    void WhileStatement::typeCheck(diagnostic::Diagnostics& diag, bool& exit)
    {
        mCondition->typeCheck(diag, exit);
        mBody->typeCheck(diag, exit);

        if (dynamic_cast<VariableDeclaration*>(mBody.get()))
        {
            diag.reportCompilerError(
                    mBody->getSourcePair().start,
                    mBody->getSourcePair().end,
                    std::format("variable declaration may not be used as body of while-statement")
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