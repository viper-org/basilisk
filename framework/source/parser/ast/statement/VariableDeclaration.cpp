// Copyright 2025 solar-mist

#include "parser/ast/statement/VariableDeclaration.h"

#include <vipir/IR/Instruction/AllocaInst.h>

namespace parser
{
    VariableDeclaration::VariableDeclaration(Scope* scope, std::string name, Type* type, ASTNodePtr initValue, SourcePair source)
        : ASTNode(scope, source, type)
        , mName(std::move(name))
        , mInitValue(std::move(initValue))
    {
        mScope->symbols.push_back(std::make_unique<Symbol>(mName, type));
        mSymbol = mScope->symbols.back().get();
    }

    vipir::Value* VariableDeclaration::codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        mSymbol->diVariable = diBuilder.createDebugVariable(mName, builder.getInsertPoint()->getParent(), mType->getDIType(), mSource.start.line, mSource.start.col);
        if (mType->isArrayType() || mType->isStructType())
        {
            auto alloca = builder.CreateAlloca(mType->getVipirType());
            //mSymbol->values.push_back(std::make_pair(builder.getInsertPoint(), alloca));
            mSymbol->values.push_back({builder.getInsertPoint(), alloca, nullptr, nullptr});
            // TODO: Check initValue here
        }
        

        if (mInitValue)
        {
            auto q1 = builder.CreateQueryAddress();
            vipir::Value* initValue = mInitValue->dcodegen(builder, diBuilder, module, diag);
            mSymbol->values.push_back({builder.getInsertPoint(), initValue, q1, nullptr});
        }


        return nullptr;
    }

    void VariableDeclaration::typeCheck(diagnostic::Diagnostics& diag, bool& exit)
    {
        if (!mType)
        {
            if (!mInitValue)
            {
                diag.reportCompilerError(
                    mSource.start,
                    mSource.end,
                    std::format("object '{}{}{}' has unknown type",
                        fmt::bold, mName, fmt::defaults)
                );
                exit = true;
                mType = Type::Get("error-type");
                return;
            }

            mInitValue->typeCheck(diag, exit);
            mType = mInitValue->getType();
            mSymbol->type = mType; // This needs to be set again as it was set to nullptr in the constructor
        }

        if (mInitValue)
        {
            mInitValue->typeCheck(diag, exit);

            if (mInitValue->getType() != mType)
            {
                if (mInitValue->canImplicitCast(diag, mType))
                {
                    mInitValue = Cast(mInitValue, mType);
                }
                else
                {
                    diag.reportCompilerError(
                        mInitValue->getSourcePair().start,
                        mInitValue->getSourcePair().end,
                        std::format("value of type '{}{}{}' is not compatible with variable of type '{}{}{}'",
                            fmt::bold, mInitValue->getType()->getName(), fmt::defaults,
                            fmt::bold, mType->getName(), fmt::defaults)
                    );
                    exit = true;
                }
            }
        }
    }
}