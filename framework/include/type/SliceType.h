// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_TYPE_SLICE_TYPE_H
#define BASILISK_FRAMEWORK_TYPE_SLICE_TYPE_H 1

#include "type/Type.h"

class SliceType : public Type
{
public:
    SliceType(Type* pointeeType);

    Type* getPointeeType() const;

    virtual int getSize() const override;
    virtual vipir::Type* getVipirType() const override;
    virtual CastLevel castTo(Type* destType) const override;
    virtual std::string getSymbolID(Type* thisType) const override;

    bool isSliceType() const override;

    static SliceType* Get(Type* pointeeType);
    static void SetDITypes();
    static void Reset();

private:
    Type* mPointeeType;
    vipir::StructType* mVipirType;
};

#endif // BASILISK_FRAMEWORK_TYPE_SLICE_TYPE_H