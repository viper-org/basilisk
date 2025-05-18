// Copyright 2025 solar-mist

#include "type/ArrayType.h"
#include "type/PointerType.h"

#include <format>
#include <algorithm>

ArrayType::ArrayType(Type* elementType, unsigned int length)
    : Type(std::format("{}[{}]", elementType->getName(), length))
    , mElementType(elementType)
    , mLength(length)
{
}

Type* ArrayType::getElementType() const
{
    return mElementType;
}

unsigned int ArrayType::getLength() const
{
    return mLength;
}

int ArrayType::getSize() const
{
    return mElementType->getSize() * mLength;
}

vipir::Type* ArrayType::getVipirType() const
{
    return vipir::Type::GetArrayType(mElementType->getVipirType(), mLength);
}

Type::CastLevel ArrayType::castTo(Type* destType) const
{
    if (destType->isPointerType())
    {
        if (static_cast<PointerType*>(destType)->getPointeeType() == mElementType)
        {
            return Type::CastLevel::Implicit;
        }
    }
    return Type::CastLevel::Disallowed;
}

std::string ArrayType::getSymbolID() const
{
    return "UNIMPLEMENTED";
}

bool ArrayType::isArrayType() const
{
    return true;
}

static std::vector<std::unique_ptr<ArrayType> > arrayTypes;
ArrayType* ArrayType::Get(Type* elementType, unsigned int length)
{
    auto it = std::find_if(arrayTypes.begin(), arrayTypes.end(), [elementType, length](const auto& type){
        return type->getElementType() == elementType && type->getLength() == length;
    });

    if (it != arrayTypes.end())
    {
        return it->get();
    }

    arrayTypes.push_back(std::make_unique<ArrayType>(elementType, length));
    return arrayTypes.back().get();
}

void ArrayType::Reset()
{
    arrayTypes.clear();
}