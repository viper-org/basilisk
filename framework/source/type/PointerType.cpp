// Copyright 2025 solar-mist

#include "type/PointerType.h"

#include <format>
#include <algorithm>

PointerType::PointerType(Type* pointeeType)
    : Type(std::format("{}*", pointeeType->getName()))
    , mPointeeType(pointeeType)
{
}

Type* PointerType::getPointeeType() const
{
    return mPointeeType;
}

int PointerType::getSize() const
{
    return 64;
}

vipir::Type* PointerType::getVipirType() const
{
    return vipir::Type::GetPointerType(mPointeeType->getVipirType());
}

Type::CastLevel PointerType::castTo(Type* destType) const
{
    if (destType->isPointerType())
    {
        if (destType == this)
        {
            return Type::CastLevel::Implicit;
        }
        if (mPointeeType->isVoidType())
        {
            return Type::CastLevel::Implicit;
        }
        return Type::CastLevel::Explicit;
    }
    if (destType->isIntegerType())
    {
        if (destType->getSize() == getSize())
        {
            return Type::CastLevel::Explicit;
        }
    }
    return Type::CastLevel::Disallowed;
}

std::string PointerType::getSymbolID() const
{
    return "P" + mPointeeType->getSymbolID();
}

bool PointerType::isPointerType() const
{
    return true;
}

// Maybe replace this with a hashmap?
static std::vector<std::unique_ptr<PointerType> > pointerTypes;

PointerType* PointerType::Get(Type* pointeeType)
{
    auto it = std::find_if(pointerTypes.begin(), pointerTypes.end(), [pointeeType](const auto& type){
        return type->getPointeeType() == pointeeType;
    });

    if (it != pointerTypes.end())
    {
        return it->get();
    }

    pointerTypes.push_back(std::make_unique<PointerType>(pointeeType));
    return pointerTypes.back().get();
}

void PointerType::SetDITypes()
{
    for (auto& type : pointerTypes)
    {
        type->mDiType = Type::GetDIBuilder()->createPointerType(type->mPointeeType->getDIType());
    }
}

void PointerType::Reset()
{
    pointerTypes.clear();
}