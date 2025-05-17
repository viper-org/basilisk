// Copyright 2025 solar-mist

#include "parser/ast/statement/ForStatement.h"
#include "parser/ast/statement/VariableDeclaration.h"

#include <vipir/IR/BasicBlock.h>
#include <vipir/IR/Function.h>
#include <vipir/IR/Instruction/PhiInst.h>
#include <vipir/IR/Instruction/AllocaInst.h>

#include <algorithm>

namespace parser
{
    ForStatement::ForStatement(ASTNodePtr init, ASTNodePtr condition, ASTNodePtr it, ASTNodePtr body, Scope* scope, std::string label, SourcePair source)
        : ASTNode(scope, source)
        , mInit(std::move(init))
        , mCondition(std::move(condition))
        , mIt(std::move(it))
        , mBody(std::move(body))
        , mLabel(std::move(label))
    {
    }

    vipir::Value* ForStatement::codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        vipir::BasicBlock* startBasicBlock = builder.getInsertPoint();

        //vipir::BasicBlock* conditionBasicBlock = vipir::BasicBlock::Create("", builder.getInsertPoint()->getParent());
        vipir::BasicBlock* bodyBasicBlock = vipir::BasicBlock::Create("", builder.getInsertPoint()->getParent());
        vipir::BasicBlock* itBasicBlock = vipir::BasicBlock::Create("", builder.getInsertPoint()->getParent());
        vipir::BasicBlock* mergeBasicBlock = vipir::BasicBlock::Create("", builder.getInsertPoint()->getParent());
        
        mScope->loopContext = LoopContext(itBasicBlock, mergeBasicBlock, mLabel);

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

        mInit->dcodegen(builder, diBuilder, module, diag);
        mCondition->ccodegen(builder, diBuilder, module, diag, bodyBasicBlock, mergeBasicBlock);
        //vipir::Value* precondition = mCondition->dcodegen(builder, diBuilder, module, diag);
        //builder.CreateCondBr(precondition, bodyBasicBlock, mergeBasicBlock);
        
        bodyBasicBlock->loopEnd() = mergeBasicBlock;
        itBasicBlock->loopEnd() = mergeBasicBlock;

        builder.setInsertPoint(bodyBasicBlock);
        for (auto symbol : symbols)
        {
            auto startBasicBlockValue = symbol->getLatestValue(startBasicBlock);
            if (!startBasicBlockValue || dynamic_cast<vipir::AllocaInst*>(startBasicBlockValue->value)
             || startBasicBlockValue->value->getType()->isStructType()
             || startBasicBlockValue->value->getType()->isArrayType())
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
        if (!builder.getInsertPoint()->hasTerminator())
            builder.CreateBr(itBasicBlock);
        builder.getInsertPoint()->loopEnd() = mergeBasicBlock;

        // Only codegen the iterator BB if the body branches to it
        if (builder.getInsertPoint()->successors().back() == itBasicBlock)
        {
            builder.setInsertPoint(itBasicBlock);
            mIt->dcodegen(builder, diBuilder, module, diag);
            mCondition->ccodegen(builder, diBuilder, module, diag, bodyBasicBlock, mergeBasicBlock);
            //vipir::Value* condition = mCondition->dcodegen(builder, diBuilder, module, diag);
            //builder.CreateCondBr(condition, bodyBasicBlock, mergeBasicBlock);
        }
        
        for (int i = 0; i < phis.size(); ++i)
        {
            if (!phis[i]) continue;

            int incoming = 1;
            auto startBasicBlockValue = symbols[i]->getLatestValue(startBasicBlock);
            for (auto bb : bodyBasicBlock->predecessors())
            {
                auto value = symbols[i]->getLatestValueX(bb);
                if (value && value != startBasicBlockValue && value->value != phis[i])
                {
                    phis[i]->addIncoming(value->value, bb);
                    ++incoming;
                }
            }
            if (incoming == 1)
            {
                std::erase_if(symbols[i]->values, [phi = phis[i]](auto v){
                    return v.value == phi;
                });
                builder.getInsertPoint()->getParent()->replaceAllUsesWith(phis[i], startBasicBlockValue->value);
                phis[i]->eraseFromParent();
            }
            else
            {
                auto q2 = builder.CreateQueryAddress();
                symbols[i]->values.back().end = q2;
                //symbols[i]->values.push_back({mergeBasicBlock, phis[i], q2, nullptr});
            }
        }
        
        builder.setInsertPoint(mergeBasicBlock);
        for (auto symbol : symbols)
        {
            auto startBasicBlockValue = symbol->getLatestValue(startBasicBlock);
            if (!startBasicBlockValue || dynamic_cast<vipir::AllocaInst*>(startBasicBlockValue->value))
            {
                continue;
            }

            std::vector<std::pair<SymbolValue*, vipir::BasicBlock*> > values;
            for (auto pred : mergeBasicBlock->predecessors())
            {
                auto value = symbol->getLatestValueX(pred);
                if (value) values.push_back({value, pred});
            }

            if (values.size() > 1)
            {
                auto phi = builder.CreatePhi(symbol->type->getVipirType());
                for (auto value : values)
                {
                    phi->addIncoming(value.first->value, value.second);
                }

                auto q2 = builder.CreateQueryAddress();
                symbol->getLatestValue()->end = q2;
                symbol->values.push_back({mergeBasicBlock, phi, q2, nullptr});
            }
        }

        return nullptr;
    }

    std::vector<ASTNode*> ForStatement::getChildren()
    {
        return { mInit.get(), mCondition.get(), mIt.get(), mBody.get() };
    }

    void ForStatement::typeCheck(diagnostic::Diagnostics& diag, bool& exit)
    {
        mInit->typeCheck(diag, exit);
        mCondition->typeCheck(diag, exit);
        mIt->typeCheck(diag, exit);
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