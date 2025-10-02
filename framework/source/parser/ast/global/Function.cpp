// Copyright 2025 solar-mist

#include "parser/ast/global/Function.h"

#include "parser/ast/statement/ReturnStatement.h"

#include "type/PointerType.h"

#include <vipir/IR/Function.h>

#include <vipir/IR/Instruction/AllocaInst.h>

#include <vipir/IR/Constant/ConstantInt.h>

#include <vipir/Type/FunctionType.h>

#include <functional>

namespace parser
{
    FunctionArgument::FunctionArgument(Type* type, std::string name)
        : type(type)
        , name(std::move(name))
    {
    }
    

    Function::Function(bool exported, Type* implType, std::string name, FunctionType* functionType, std::vector<FunctionArgument> arguments, ScopePtr ownScope, bool external, std::vector<ASTNodePtr> body, SourcePair source, SourcePair blockEnd)
        : ASTNode(ownScope->parent, source, functionType)
        , mImplType(implType)
        , mName(std::move(name))
        , mArguments(std::move(arguments))
        , mExternal(external)
        , mBody(std::move(body))
        , mBlockEnd(std::move(blockEnd))
        , mOwnScope(std::move(ownScope))
    {
        if (mImplType)
        {
            mArguments.insert(mArguments.begin(), FunctionArgument(PointerType::Get(mImplType), "this"));

            auto argTypes = functionType->getArgumentTypes();
            argTypes.insert(argTypes.begin(), PointerType::Get(mImplType));
            functionType = FunctionType::Create(functionType->getReturnType(), std::move(argTypes));
            mType = functionType;
        }

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
        auto function = static_cast<vipir::Function*>(mSymbol->values.front().value);

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
            auto diVariable = diBuilder.createParameterVariable(argument.name, function, argument.type->getDIType());
            argument.symbol->diVariable = diVariable;

            auto arg = function->getArgument(index++);
            if(false)//if (argument.type->isArrayType() || argument.type->isStructType() || argument.type->isSliceType())
            {
                auto alloca = builder.CreateAlloca(argument.type->getVipirType());
                builder.CreateStore(alloca, arg);
                argument.symbol->values.push_back({entryBB, alloca, nullptr, nullptr});
            }
            else
            {
                auto q1 = builder.CreateQueryAddress();
                argument.symbol->values.push_back({entryBB, arg, q1, nullptr});
            }
        }

        for (auto& node : mBody)
        {
            node->dcodegen(builder, diBuilder, module, diag);
        }
        if (!builder.getInsertPoint()->hasTerminator())
        {
            auto retType = static_cast<FunctionType*>(mType)->getReturnType();
            if (retType->isVoidType())
            {
                builder.CreateRet(nullptr);
            }
            else if (retType->isIntegerType())
            {
                builder.CreateRet(vipir::ConstantInt::Get(module, 0, retType->getVipirType()));
            }
        }

        return function;
    }

    void Function::setEmittedValue(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        vipir::FunctionType* functionType = dynamic_cast<vipir::FunctionType*>(mType->getVipirType());

        // This should never happen but good to check just in case
        if (!functionType)
        {
            diag.fatalError("mType of parser::Function is not a function type.");
            std::exit(1);
        }

        auto mangledName = MangleName(mName, mImplType, static_cast<FunctionType*>(mType));

        vipir::Function* function = vipir::Function::Create(functionType, module, mangledName, false);

        mSymbol->values.push_back({nullptr, function, nullptr, nullptr});
    }

    std::vector<ASTNode*> Function::getChildren()
    {
        std::vector<ASTNode*> children;
        for (auto& node : mBody)
        {
            children.push_back(node.get());
        }
        return children;
    }

    void Function::typeCheck(diagnostic::Diagnostics& diag, bool& exit)
    {
        if (static_cast<FunctionType*>(mType)->getReturnType() == Type::Get("error-type"))
        {
            std::function<ReturnStatement*(ASTNode*)> process;
            process = [&process](ASTNode* node) -> ReturnStatement* {
                if (auto ret = dynamic_cast<ReturnStatement*>(node))
                {
                    return ret;
                }
                for (auto child : node->getChildren())
                {
                    if (auto ret = process(child))
                    {
                        return ret;
                    }
                }
                return nullptr;
            };
            std::vector<Type*> parameterTypes;
            Type* returnType = nullptr;
            for (auto& argument : mArguments)
            {
                parameterTypes.push_back(argument.type);
            }
            for (auto& node : mBody)
            {
                if (auto ret = process(node.get()))
                {
                    if (ret->getChildren().empty())
                    {
                        returnType = Type::Get("void");
                    }
                    else
                    {
                        returnType = ret->getChildren()[0]->getType();
                    }
                    break;
                }
            }
            if (!returnType)
            {
                diag.reportCompilerError(
                    mSource.start,
                    mSource.end,
                    std::format("could not deduce return type")
                );
                exit = true;
            }
            else
            {
                auto functionType = FunctionType::Create(returnType, std::move(parameterTypes));
                mType = functionType;
                mSymbol->type = functionType;
                mOwnScope->currentReturnType = returnType;
            }
        }

        for (auto& node : mBody)
        {
            node->typeCheck(diag, exit);
        }
    }

    ASTNodePtr Function::cloneExternal(Scope* in)
    {
        auto ownScope = std::make_unique<Scope>(in);
        auto functionType = static_cast<FunctionType*>(mType);
        return std::make_unique<Function>(false, mImplType, mName, functionType, mArguments, std::move(ownScope), true, std::vector<ASTNodePtr>(), mSource, mBlockEnd);
    }

    std::string Function::getName() const
    {
        return mName;
    }


    std::string Function::MangleName(std::string name, Type* implType, FunctionType* functionType)
    {
        // puts is just here for testing on windows, it will be removed later
        if (name == "_start" || name == "main" || name == "puts") return name;

        std::string ret = "_F";
        if (implType) ret += "I" + implType->getSymbolID(nullptr);

        ret += std::to_string(name.length());
        ret += name;

        for (auto& arg : functionType->getArgumentTypes())
        {
            ret += arg->getSymbolID(nullptr);
        }

        return ret;
    }
}