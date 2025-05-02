// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_TYPE_POINTER_TYPE_H
#define BASILISK_FRAMEWORK_TYPE_POINTER_TYPE_H 1

#include "type/Type.h"

class PointerType : public Type
{
public:
    PointerType(Type* pointeeType);

    Type* getPointeeType() const;

    virtual int getSize() const override;
    virtual vipir::Type* getVipirType() const override;
    virtual CastLevel castTo(Type* destType) const override;

    bool isPointerType() const override;

    static PointerType* Get(Type* pointeeType);
    static void SetDITypes();
    static void Reset();

private:
    Type* mPointeeType;
};

#endif // BASILISK_FRAMEWORK_TYPE_POINTER_TYPE_H