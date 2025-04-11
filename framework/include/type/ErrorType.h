// Copyright 2024 solar-mist

#ifndef BASILISK_FRAMEWORK_TYPE_ERROR_TYPE_H
#define BASILISK_FRAMEWORK_TYPE_ERROR_TYPE_H 1

#include "type/Type.h"

class ErrorType : public Type
{
public:
    ErrorType();

    virtual int getSize() const override;
    virtual vipir::Type* getVipirType() const override;
    virtual CastLevel castTo(Type* destType) const override;
};

#endif // BASILISK_FRAMEWORK_TYPE_ERROR_TYPE_H