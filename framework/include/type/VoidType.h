// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_TYPE_VOID_TYPE_H
#define BASILISK_FRAMEWORK_TYPE_VOID_TYPE_H 1

#include "type/Type.h"

class VoidType : public Type
{
public:
    VoidType();

    virtual int getSize() const override;
    virtual vipir::Type* getVipirType() const override;
    virtual CastLevel castTo(Type* destType) const override;

    bool isVoidType() const override;
};

#endif // BASILISK_FRAMEWORK_TYPE_VOID_TYPE_H