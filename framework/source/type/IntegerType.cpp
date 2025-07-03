// Copyright 2025 solar-mist

#include "type/IntegerType.h"

#include "diagnostic/Diagnostic.h"

#include <format>

IntegerType::IntegerType(int bits, bool isSigned)
    : Type(std::format("{}{}", (isSigned ? "i" : "u"), bits))
    , mBits(bits)
    , mSigned(isSigned)
{
}

int IntegerType::getSize() const
{
    return mBits;
}

vipir::Type* IntegerType::getVipirType() const
{
    return vipir::Type::GetIntegerType(mBits);
}

Type::CastLevel IntegerType::castTo(Type* destType) const
{
    if (destType->isIntegerType())
    {
        if (destType->getSize() < mBits)
        {
            return Type::CastLevel::ImplicitWarning;
        }
        return Type::CastLevel::Implicit;
    }
    else if (destType->isBooleanType())
    {
        return Type::CastLevel::ImplicitWarning;
    }
    else if (destType->isPointerType())
    {
	return Type::CastLevel::Explicit;
    }
    return Type::CastLevel::Disallowed;
}

std::string IntegerType::getImplicitCastWarning(Type* destType) const
{
    return std::format("potential loss of data casting '{}{}{}' to '{}{}{}'",
        fmt::bold, mName, fmt::defaults,
        fmt::bold, destType->getName(), fmt::defaults);
}

std::string IntegerType::getSymbolID(Type*) const
{
    if (mSigned)
    {
        switch (mBits)
        {
            case 8: return "c";
            case 16: return "s";
            case 32: return "i";
            case 64: return "l";
        }
    }
    else
    {
        switch (mBits)
        {
            case 8: return "b";
            case 16: return "w";
            case 32: return "d";
            case 64: return "q";
        }
    }
    return ""; // unreachable
}

bool IntegerType::isIntegerType() const
{
    return true;
}

bool IntegerType::isSigned() const
{
    return mSigned;
}
