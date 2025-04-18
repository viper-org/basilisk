// Copyright 2025 solar-mist

#include "parser/ast/expression/CallExpression.h"
#include "parser/ast/expression/VariableExpression.h"

#include "type/FunctionType.h"

#include <vipir/Module.h>

#include <vipir/IR/Function.h>
#include <vipir/IR/Instruction/CallInst.h>
#include <vipir/IR/Instruction/LoadInst.h>
#include <vipir/IR/Instruction/GEPInst.h>
#include <vipir/IR/Instruction/AddrInst.h>

namespace parser
{
    CallExpression::CallExpression(Scope* scope, ASTNodePtr callee, std::vector<ASTNodePtr> parameters, SourcePair source)
        : ASTNode(scope, std::move(source))
        , mCallee(std::move(callee))
        , mParameters(std::move(parameters))
    {
    }

    vipir::Value* CallExpression::codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        vipir::Value* callee = mFunction->getLatestValue();

        std::vector<vipir::Value*> parameters;
        for (auto& parameter : mParameters)
        {
            parameters.push_back(parameter->codegen(builder, diBuilder, module, diag));
        }

        return builder.CreateCall(static_cast<vipir::Function*>(callee), std::move(parameters));
    }

    void CallExpression::typeCheck(diagnostic::Diagnostics& diag, bool& exit)
    {
        mCallee->typeCheck(diag, exit);
        for (auto& parameter : mParameters)
        {
            parameter->typeCheck(diag, exit);
        }
        if (auto var = dynamic_cast<VariableExpression*>(mCallee.get()))
        {
            mFunction = mScope->resolveSymbol(var->getName());
            if (mFunction && !mFunction->type->isFunctionType())
            {
                diag.reportCompilerError(
                    mSource.start,
                    mSource.end,
                    std::format("object is not callable")
                    // TODO: Print "no matching function for call to ..."
                );
                exit = true;
                mFunction = nullptr;
            }
        }
        else
        {
            diag.reportCompilerError(
                mSource.start,
                mSource.end,
                std::format("object is not callable")
            );
            exit = true;
        }

        if (!mFunction)
        {
            mType = Type::Get("error-type");
        }
        else
        {
            auto functionType = static_cast<FunctionType*>(mFunction->type);
            mType = functionType->getReturnType();
            unsigned int index = 0;
            for (auto& parameter : mParameters)
            {
                auto argumentType = functionType->getArgumentTypes()[index];
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
                                fmt::bold, mFunction->name, fmt::defaults)
                        );
                        exit = true;
                        mType = Type::Get("error-type");
                    }
                }
            }
        }
    }
}