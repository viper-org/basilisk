// Copyright 2025 solar-mist

#include "type/ErrorType.h"

ErrorType::ErrorType(int size)
    : Type("error-type")
    , mSize(size)
{
}

int ErrorType::getSize() const
{
    return mSize;
}

Type::CastLevel ErrorType::castTo(Type* destType) const
{
    return CastLevel::Disallowed;
}

vipir::Type* ErrorType::getVipirType() const
{
    return vipir::Type::GetVoidType();
}

std::string ErrorType::getSymbolID(Type*) const
{
    return "stray error type in program";
}