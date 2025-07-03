// Copyright 2025 solar-mist

#include "type/VoidType.h"

VoidType::VoidType()
    : Type("void")
{
}

int VoidType::getSize() const
{
    return 0;
}

Type::CastLevel VoidType::castTo(Type* destType) const
{
    return CastLevel::Disallowed;
}

vipir::Type* VoidType::getVipirType() const
{
    return vipir::Type::GetVoidType();
}

std::string VoidType::getSymbolID(Type*) const
{
    return "v";
}

bool VoidType::isVoidType() const
{
    return true;
}