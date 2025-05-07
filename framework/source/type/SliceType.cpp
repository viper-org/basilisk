// Copyright 2025 solar-mist

#include "type/SliceType.h"

#include <format>
#include <algorithm>

SliceType::SliceType(Type* pointeeType)
    : Type(std::format("{}[]", pointeeType->getName()))
    , mPointeeType(pointeeType)
{
    auto type = vipir::Type::GetStructType({
        vipir::Type::GetPointerType(pointeeType->getVipirType()),
        vipir::Type::GetIntegerType(64)
    });
    mVipirType = static_cast<vipir::StructType*>(type);
}

Type* SliceType::getPointeeType() const
{
    return mPointeeType;
}

int SliceType::getSize() const
{
    return 128;
}

vipir::Type* SliceType::getVipirType() const
{
    return mVipirType;
}

Type::CastLevel SliceType::castTo(Type* destType) const
{
    if (destType->isSliceType())
    {
        if (destType == this)
        {
            return Type::CastLevel::Implicit;
        }
        if (static_cast<SliceType*>(destType)->getPointeeType()->isVoidType())
        {
            return Type::CastLevel::Implicit;
        }
    }
    return Type::CastLevel::Disallowed;
}

bool SliceType::isSliceType() const
{
    return true;
}

// Maybe replace this with a hashmap?
static std::vector<std::unique_ptr<SliceType> > sliceTypes;

SliceType* SliceType::Get(Type* pointeeType)
{
    auto it = std::find_if(sliceTypes.begin(), sliceTypes.end(), [pointeeType](const auto& type){
        return type->getPointeeType() == pointeeType;
    });

    if (it != sliceTypes.end())
    {
        return it->get();
    }

    sliceTypes.push_back(std::make_unique<SliceType>(pointeeType));
    return sliceTypes.back().get();
}

void SliceType::SetDITypes()
{
    for (auto& type : sliceTypes)
    {
        // TODO: Line number info?
        type->mDiType = Type::GetDIBuilder()->createStructureType(type->mName, type->mVipirType, 0, 0);
    }
}

void SliceType::Reset()
{
    sliceTypes.clear();
}