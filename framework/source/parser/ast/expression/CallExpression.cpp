// Copyright 2025 solar-mist

#include "parser/ast/expression/CallExpression.h"
#include "parser/ast/expression/MemberAccess.h"
#include "parser/ast/expression/VariableExpression.h"

#include "type/FunctionType.h"

#include <vipir/Module.h>

#include <vipir/IR/Function.h>
#include <vipir/IR/Instruction/CallInst.h>
#include <vipir/IR/Instruction/LoadInst.h>
#include <vipir/IR/Instruction/GEPInst.h>
#include <vipir/IR/Instruction/AddrInst.h>

#include <algorithm>

namespace parser
{
    CallExpression::CallExpression(Scope* scope, ASTNodePtr callee, std::vector<ASTNodePtr> parameters, SourcePair source)
        : ASTNode(scope, std::move(source))
        , mCallee(std::move(callee))
        , mParameters(std::move(parameters))
        , mIsMemberFunction(false)
    {
    }

    vipir::Value* CallExpression::codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        vipir::Value* callee = mBestViableFunction->getLatestValue()->value;

        std::vector<vipir::Value*> parameters;

        if (auto memberAccess = dynamic_cast<MemberAccess*>(mCallee.get()))
        {
            auto value = memberAccess->mStruct->dcodegen(builder, diBuilder, module, diag);
            if (memberAccess->mStruct->getType()->isStructType())
            {
                vipir::Value* self = vipir::getPointerOperand(value);

                vipir::Instruction* instruction = static_cast<vipir::Instruction*>(value);
                instruction->eraseFromParent();

                if (dynamic_cast<vipir::GEPInst*>(self))
                {
                    value = self;
                }
                else
                {
                    value = builder.CreateAddrOf(self);
                }
                parameters.push_back(value);
            }
            else
            {
                parameters.push_back(value);
            }
        }

        for (auto& parameter : mParameters)
        {
            parameters.push_back(parameter->dcodegen(builder, diBuilder, module, diag));
        }

        return builder.CreateCall(static_cast<vipir::Function*>(callee), std::move(parameters));
    }

    std::vector<ASTNode*> CallExpression::getChildren()
    {
        std::vector<ASTNode*> children;
        children.push_back(mCallee.get());
        for (auto& parameter : mParameters)
        {
            children.push_back(parameter.get());
        }
        return children;
    }

    void CallExpression::typeCheck(diagnostic::Diagnostics& diag, bool& exit)
    {
        mCallee->typeCheck(diag, exit);
        for (auto& parameter : mParameters)
        {
            parameter->typeCheck(diag, exit);
        }
        mBestViableFunction = getBestViableFunction(diag, exit);
        if (!mBestViableFunction)
        {
            exit = true;
            mType = Type::Get("error-type");
        }
        else
        {
            auto functionType = static_cast<FunctionType*>(mBestViableFunction->type);
            mType = functionType->getReturnType();
            unsigned int index = 0;
            for (auto& parameter : mParameters)
            {
                auto argumentType = functionType->getArgumentTypes()[index + mIsMemberFunction];
                ++index;
                if (parameter->getType() != argumentType)
                {
                    if (parameter->canImplicitCast(diag, argumentType))
                    {
                        parameter = Cast(parameter, argumentType);
                    }
                    else
                    {
                        diag.reportCompilerError(
                            mSource.start,
                            mSource.end,
                            std::format("no matching function for call to '{}{}(){}'",
                                fmt::bold, mBestViableFunction->name, fmt::defaults)
                        );
                        exit = true;
                        mType = Type::Get("error-type");
                    }
                }
            }
        }
    }


    struct ViableFunction
    {
        Symbol* symbol;
        int score;
        bool disallowed;
    };

    Symbol* CallExpression::getBestViableFunction(diagnostic::Diagnostics& diag, bool& exit)
    {
        // TODO: Check for member access
        if (dynamic_cast<VariableExpression*>(mCallee.get()) || dynamic_cast<MemberAccess*>(mCallee.get()))
        {
            std::vector<Symbol*> candidateFunctions;
            std::string errorName;

            if (auto var = dynamic_cast<VariableExpression*>(mCallee.get()))
            {
                candidateFunctions = mScope->getCandidateFunctions(var->getName());
                errorName = var->getName();
            }
            else if (auto memberAccess = dynamic_cast<MemberAccess*>(mCallee.get()))
            {
                mIsMemberFunction = true;
                candidateFunctions = mScope->getCandidateFunctions(memberAccess->mId);
                errorName = std::format("{}::{}", memberAccess->mStructType->getName(), memberAccess->mId);
            }
            
            // TODO: Check for function pointer

            // Find all viable functions
            for (auto it = candidateFunctions.begin(); it != candidateFunctions.end();)
            {
                auto candidate = *it;
                if (!candidate->type->isFunctionType()) it = candidateFunctions.erase(it);
                else
                {
                    auto functionType = static_cast<FunctionType*>(candidate->type);
                    auto arguments = functionType->getArgumentTypes();
                    if (arguments.size() != (mParameters.size() + mIsMemberFunction))
                        it = candidateFunctions.erase(it);
                    else
                        ++it;
                }
            }

            std::vector<ViableFunction> viableFunctions;
            for (auto& candidate : candidateFunctions)
            {
                auto functionType = static_cast<FunctionType*>(candidate->type);
                int score = 0;
                bool disallowed = false;
                size_t i = 0;
                for (;i < mParameters.size(); ++i)
                {
                    auto castLevel = mParameters[i]->getType()->castTo(functionType->getArgumentTypes()[i+mIsMemberFunction]);
                    int multiplier = 0;
                    if (mParameters[i]->getType() == functionType->getArgumentTypes()[i]) multiplier = 0;
                    else if (castLevel == Type::CastLevel::Implicit) multiplier = 1;
                    else if (castLevel == Type::CastLevel::ImplicitWarning) multiplier = 2;
                    else disallowed = true;
                    score += multiplier * (mParameters.size() - i); // Weight earlier scores more
                }
                if (!disallowed)
                {
                    viableFunctions.push_back({candidate, score});
                }
            }

            if (viableFunctions.empty())
            {
                diag.reportCompilerError(
                    mSource.start,
                    mSource.end,
                    std::format("no matching function for call to '{}{}(){}'",
                        fmt::bold, errorName, fmt::defaults)
                );
                return nullptr;
            }

            std::sort(viableFunctions.begin(), viableFunctions.end(), [](const auto& lhs, const auto& rhs){
                return lhs.score < rhs.score;
            });
            if (viableFunctions.size() >= 2)
            {
                if (viableFunctions[0].score == viableFunctions[1].score)
                {
                    diag.reportCompilerError(
                        mSource.start,
                        mSource.end,
                        std::format("call to '{}{}(){}' is ambiguous",
                            fmt::bold, errorName, fmt::defaults)
                    );
                    return nullptr;
                }
            }
            return viableFunctions.front().symbol;
        }
        return nullptr;
    }
}
