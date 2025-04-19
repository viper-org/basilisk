// Copyright 2025 solar-mist

#include "parser/ast/global/Function.h"

#include <vipir/Type/FunctionType.h>
#include <vipir/IR/Function.h>

namespace parser
{
    FunctionArgument::FunctionArgument(Type* type, std::string name)
        : type(type)
        , name(std::move(name))
    {
    }
    

    Function::Function(bool exported, std::string name, FunctionType* functionType, std::vector<FunctionArgument> arguments, ScopePtr ownScope, bool external, std::vector<ASTNodePtr> body, SourcePair source, SourcePair blockEnd)
        : ASTNode(ownScope->parent, source, functionType)
        , mName(std::move(name))
        , mArguments(std::move(arguments))
        , mExternal(external)
        , mBody(std::move(body))
        , mBlockEnd(std::move(blockEnd))
        , mOwnScope(std::move(ownScope))
    {
        mOwnScope->currentReturnType = functionType->getReturnType();
        mScope->symbols.push_back(std::make_unique<Symbol>(mName, functionType));
        mSymbol = mScope->symbols.back().get();
        mSymbol->exported = exported;

        for (auto& argument : mArguments)
        {
            mOwnScope->symbols.push_back(std::make_unique<Symbol>(argument.name, argument.type));
            argument.symbol = mOwnScope->symbols.back().get();
        }
    }

    vipir::Value* Function::codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        vipir::FunctionType* functionType = dynamic_cast<vipir::FunctionType*>(mType->getVipirType());

        // This should never happen but good to check just in case
        if (!functionType)
        {
            diag.fatalError("mType of parser::Function is not a function type.");
            std::exit(1);
        }

        vipir::Function* function = vipir::Function::Create(functionType, module, mName, false);

        mSymbol->values.push_back({nullptr, function, nullptr, nullptr});

        diBuilder.setDebugType(function, static_cast<FunctionType*>(mType)->getReturnType()->getDIType());

        if (mExternal)
        {
            assert(mBody.empty());
            return function;
        }

        diBuilder.setSourceInfo(function, mSource.start.line, mSource.start.col, mBlockEnd.start.line, mBlockEnd.start.col);

        vipir::BasicBlock* entryBB = vipir::BasicBlock::Create("", function);
        builder.setInsertPoint(entryBB);
        
        unsigned int index = 0;
        for (auto& argument : mArguments)
        {
            auto arg = function->getArgument(index++);
            argument.symbol->values.push_back({entryBB, arg, nullptr, nullptr});
        }

        for (auto& node : mBody)
        {
            node->dcodegen(builder, diBuilder, module, diag);
        }

        return function;
    }

    void Function::typeCheck(diagnostic::Diagnostics& diag, bool& exit)
    {
        for (auto& node : mBody)
        {
            node->typeCheck(diag, exit);
        }
    }
}