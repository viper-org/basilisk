// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_TYPE_BOOLEAN_TYPE_H
#define BASILISK_FRAMEWORK_TYPE_BOOLEAN_TYPE_H 1

#include "type/Type.h"

class BooleanType : public Type
{
public:
    BooleanType();

    virtual int getSize() const override;
    virtual vipir::Type* getVipirType() const override;
    virtual CastLevel castTo(Type* destType) const override;
    virtual std::string getSymbolID(Type* thisType) const override;

    bool isBooleanType() const override;
};

#endif // BASILISK_FRAMEWORK_TYPE_BOOLEAN_TYPE_H