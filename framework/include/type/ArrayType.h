// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_TYPE_ARRAY_TYPE_H
#define BASILISK_FRAMEWORK_TYPE_ARRAY_TYPE_H 1

#include "type/Type.h"

class ArrayType : public Type
{
public:
    ArrayType(Type* elementType, unsigned int length);

    Type* getElementType() const;
    unsigned int getLength() const;

    virtual int getSize() const override;
    virtual vipir::Type* getVipirType() const override;
    virtual CastLevel castTo(Type* destType) const override;
    virtual std::string getSymbolID(Type* thisType) const override;

    bool isArrayType() const override;

    static ArrayType* Get(Type* elementType, unsigned int length);
    static void Reset();

private:
    Type* mElementType;
    unsigned int mLength;
};

#endif // BASILISK_FRAMEWORK_TYPE_ARRAY_TYPE_H