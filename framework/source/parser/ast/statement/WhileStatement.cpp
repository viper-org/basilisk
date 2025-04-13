// Copyright 2025 solar-mist

#include "parser/ast/statement/WhileStatement.h"
#include "parser/ast/statement/VariableDeclaration.h"

#include <vipir/IR/BasicBlock.h>
#include <vipir/IR/Instruction/PhiInst.h>

#include <algorithm>

namespace parser
{
    WhileStatement::WhileStatement(ASTNodePtr condition, ASTNodePtr body, Scope* scope, SourcePair source)
        : ASTNode(scope, source)
        , mCondition(std::move(condition))
        , mBody(std::move(body))
    {
    }

    vipir::Value* WhileStatement::codegen(vipir::IRBuilder& builder, vipir::Module& module, diagnostic::Diagnostics& diag)
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

        vipir::Value* precondition = mCondition->codegen(builder, module, diag);
        builder.CreateCondBr(precondition, bodyBasicBlock, mergeBasicBlock);

        bodyBasicBlock->loopEnd() = mergeBasicBlock;

        builder.setInsertPoint(bodyBasicBlock);
        for (auto symbol : symbols)
        {
            auto startBasicBlockValue = symbol->getLatestValue(startBasicBlock);
            if (!startBasicBlockValue) continue;
            auto phi = builder.CreatePhi(symbol->type->getVipirType());
            phi->addIncoming(startBasicBlockValue, startBasicBlock);
            phis.push_back(phi);

            symbol->values.push_back(std::make_pair(bodyBasicBlock, phi));
        }
        mBody->codegen(builder, module, diag);
        vipir::Value* condition = mCondition->codegen(builder, module, diag);
        builder.CreateCondBr(condition, bodyBasicBlock, mergeBasicBlock);
        
        for (int i = 0; i < phis.size(); ++i)
        {
            auto bodyBasicBlockValue = symbols[i]->getLatestValue(bodyBasicBlock);
            auto startBasicBlockValue = symbols[i]->getLatestValue(startBasicBlock);
            if (bodyBasicBlockValue && bodyBasicBlockValue != startBasicBlockValue)
            {
                if (!dynamic_cast<vipir::PhiInst*>(bodyBasicBlockValue))
                {
                    phis[i]->addIncoming(bodyBasicBlockValue, bodyBasicBlock);
                }
                else
                {
                    // Latest value needs to be updated
                    auto it = std::find_if(symbols[i]->values.begin(), symbols[i]->values.end(), [bodyBasicBlockValue](const auto& value) {
                        return value.second == bodyBasicBlockValue;
                    });
                    symbols[i]->values.erase(it);
                    phis[i]->eraseFromParent();
                }
            }
            else
            {
                phis[i]->eraseFromParent();
            }
        }

        builder.setInsertPoint(mergeBasicBlock);


        for (auto symbol : symbols)
        {
            auto bodyBasicBlockValue = symbol->getLatestValue(bodyBasicBlock);
            auto startBasicBlockValue = symbol->getLatestValue(startBasicBlock);
            if (bodyBasicBlockValue && bodyBasicBlockValue != startBasicBlockValue)
            {
                auto phi = builder.CreatePhi(symbol->type->getVipirType());
                phi->addIncoming(bodyBasicBlockValue, bodyBasicBlock);

                // Due to the other phi node in the body, we shouldn't need to add this as an incoming
                // because of how phi codegen works
                //phi->addIncoming(startBasicBlockValue, startBasicBlock);

                symbol->values.push_back(std::make_pair(mergeBasicBlock, phi));
            }
        }

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