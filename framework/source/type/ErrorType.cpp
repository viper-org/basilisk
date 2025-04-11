// Copyright 2025 solar-mist

#include "type/ErrorType.h"

ErrorType::ErrorType()
    : Type("error-type")
{
}

int ErrorType::getSize() const
{
    return 0;
}

Type::CastLevel ErrorType::castTo(Type* destType) const
{
    return CastLevel::Disallowed;
}

vipir::Type* ErrorType::getVipirType() const
{
    return vipir::Type::GetVoidType();
}