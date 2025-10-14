// Copyright 2025 solar-mist

#include "parser/ast/expression/VariableExpression.h"

#include <vipir/IR/Function.h>
#include <vipir/IR/GlobalVar.h>
#include <vipir/IR/Instruction/LoadInst.h>
#include <vipir/IR/Instruction/AllocaInst.h>
#include <vipir/IR/Instruction/GEPInst.h>

#include <cmath>

namespace parser
{
    VariableExpression::VariableExpression(Scope* scope, std::string name, SourcePair source)
        : ASTNode(scope, source)
        , mName(std::move(name))
    {
    }

    vipir::Value* VariableExpression::codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        Symbol* symbol = mScope->resolveSymbol(mName);

        if (symbol->constant)
        {
            if (symbol->values.size() != 1)
            {
                diag.reportCompilerError(
                    mSource.start,
                    mSource.end,
                    "just assert"
                );
                std::exit(1);
            }

            return symbol->values[0].value;
        }

        auto latestValue = symbol->getLatestValue(builder.getInsertPoint());
        if (!latestValue)
        {
            if (symbol->values.size() == 1)
            {
                if (dynamic_cast<vipir::GlobalVar*>(symbol->values[0].value))
                {
                    return builder.CreateLoad(symbol->values[0].value);
                }
            }
            else if (symbol->values.size() == 0)
            {
                // This should only happen in the case of an uninitialized variable having its address taken,
                // so it will be handled in UnaryExpression
				return nullptr;
            }
        }

        if (dynamic_cast<vipir::AllocaInst*>(latestValue->value) || dynamic_cast<vipir::GlobalVar*>(latestValue->value))
        {
            return builder.CreateLoad(latestValue->value);
        }
        return latestValue->value;
    }

    void VariableExpression::typeCheck(diagnostic::Diagnostics& diag, bool& exit)
    {
        Symbol* symbol = mScope->resolveSymbol(mName);

        if (!symbol)
        {
            diag.reportCompilerError(
                mSource.start,
                mSource.end,
                std::format("undeclared identifier '{}{}{}'",
                    fmt::bold, mName, fmt::defaults)
            );
            exit = true;
            mType = Type::Get("error-type");
        }
        else
        {
            mType = symbol->type;
        }
    }

    ASTNodePtr VariableExpression::cloneExternal(Scope* in)
    {
        return std::make_unique<VariableExpression>(in, mName, mSource);
    }

    std::string VariableExpression::getName()
    {
        return mName;
    }
}