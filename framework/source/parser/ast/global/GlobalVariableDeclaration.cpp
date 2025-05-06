// Copyright 2025 solar-mist

#include "parser/ast/global/GlobalVariableDeclaration.h"

#include <vipir/IR/GlobalVar.h>
#include <vipir/Module.h>
#include <vipir/IR/Instruction/AllocaInst.h>
#include <vipir/IR/Instruction/AddrInst.h>

namespace parser
{
    GlobalVariableDeclaration::GlobalVariableDeclaration(Scope* scope, std::string name, Type* type, ASTNodePtr initValue, bool exported, bool constant, SourcePair source)
        : ASTNode(scope, source, type)
        , mName(std::move(name))
        , mInitValue(std::move(initValue))
    {
        mScope->symbols.push_back(std::make_unique<Symbol>(mName, type));
        mSymbol = mScope->symbols.back().get();
        mSymbol->exported = exported;
        mSymbol->constant = constant;
    }

    vipir::Value* GlobalVariableDeclaration::codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        if (mSymbol->constant)
        {
            if (!mInitValue)
            {
                diag.reportCompilerError(
                        mSource.start,
                        mSource.end,
                        "constant declaration has no initial value"
                );
                std::exit(1);
            }

            mSymbol->values.push_back({nullptr, mInitValue->dcodegen(builder, diBuilder, module, diag), nullptr, nullptr});
        }
        else
        {
            // TODO: Add debug info for global variables
            vipir::GlobalVar* globalVar = module.createGlobalVar(mType->getVipirType());

            if (mInitValue)
                globalVar->setInitialValue(mInitValue->dcodegen(builder, diBuilder, module, diag));
            else
                globalVar->setInitialValue(nullptr);

            mSymbol->values.push_back({nullptr, globalVar, nullptr, nullptr});
        }

        return nullptr;
    }

    std::vector<ASTNode*> GlobalVariableDeclaration::getChildren()
    {
        if (mInitValue)
            return {mInitValue.get()};
        
        return {};
    }

    void GlobalVariableDeclaration::typeCheck(diagnostic::Diagnostics& diag, bool& exit)
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