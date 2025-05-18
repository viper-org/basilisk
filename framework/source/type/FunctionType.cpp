// Copyright 2025 solar-mist

#include "type/FunctionType.h"

#include <vipir/Type/FunctionType.h>

#include <algorithm>
#include <format>

FunctionType::FunctionType(Type* returnType, std::vector<Type*> argumentTypes)
    : Type(std::format("{}(", returnType->getName()))
    , mArgumentTypes(std::move(argumentTypes))
    , mReturnType(returnType)
{
    if (!mArgumentTypes.empty())
    {
        for (int i = 0; i < mArgumentTypes.size()-1; ++i)
        {
            mName += std::format("{}, ", mArgumentTypes[i]->getName());
        }
        mName += std::format("{})", mArgumentTypes.back()->getName());
    }
    else
    {
        mName += ")";
    }
}

Type* FunctionType::getReturnType() const
{
    return mReturnType;
}

const std::vector<Type*>& FunctionType::getArgumentTypes() const
{
    return mArgumentTypes;
}

int FunctionType::getSize() const
{
    return 0;
}

vipir::Type* FunctionType::getVipirType() const
{
    std::vector<vipir::Type*> argumentTypes;
    for (auto& argumentType : mArgumentTypes)
    {
        argumentTypes.push_back(argumentType->getVipirType());
    }
    return vipir::FunctionType::Create(mReturnType->getVipirType(), std::move(argumentTypes));
}

Type::CastLevel FunctionType::castTo(Type* destType) const
{
    return CastLevel::Disallowed;
}

std::string FunctionType::getSymbolID() const
{
    return "UNIMPLEMENTED";
}

bool FunctionType::isFunctionType() const
{
    return true;
}

static std::vector<std::unique_ptr<FunctionType> > functionTypes;

FunctionType* FunctionType::Create(Type* returnType, std::vector<Type*> argumentTypes)
{
    auto it = std::find_if(functionTypes.begin(), functionTypes.end(), [returnType, &argumentTypes](const auto& type){
        return type->getReturnType() == returnType && type->getArgumentTypes() == argumentTypes;
    });

    if (it != functionTypes.end())
    {
        return it->get();
    }

    functionTypes.push_back(std::make_unique<FunctionType>(returnType, std::move(argumentTypes)));
    return functionTypes.back().get();
}

void FunctionType::Reset()
{
    functionTypes.clear();
}