// Copyright 2025 solar-mist

#include "type/BooleanType.h"

BooleanType::BooleanType()
    : Type("bool")
{
}

int BooleanType::getSize() const
{
    return 8;
}

Type::CastLevel BooleanType::castTo(Type* destType) const
{
    if (destType->isIntegerType())
    {
        return CastLevel::Implicit;
    }
    return CastLevel::Disallowed;
}

vipir::Type* BooleanType::getVipirType() const
{
    return vipir::Type::GetBooleanType();
}

std::string BooleanType::getSymbolID() const
{
    return "B";
}

bool BooleanType::isBooleanType() const
{
    return true;
}