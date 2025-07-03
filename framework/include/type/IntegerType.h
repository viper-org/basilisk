// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_TYPE_INTEGER_TYPE_H
#define BASILISK_FRAMEWORK_TYPE_INTEGER_TYPE_H 1

#include "type/Type.h"

class IntegerType : public Type
{
public:
    IntegerType(int bits, bool isSigned);

    virtual int getSize() const override;
    virtual vipir::Type* getVipirType() const override;
    virtual CastLevel castTo(Type* destType) const override;
    virtual std::string getImplicitCastWarning(Type* destType) const override;
    virtual std::string getSymbolID(Type* thisType) const override;

    bool isIntegerType() const override;

    bool isSigned() const;

private:
    int mBits;
    bool mSigned;
};

#endif // BASILISK_FRAMEWORK_TYPE_INTEGER_TYPE_H