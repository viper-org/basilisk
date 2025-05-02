// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_TYPE_FUNCTION_TYPE_H
#define BASILISK_FRAMEWORK_TYPE_FUNCTION_TYPE_H 1

#include "type/Type.h"

class FunctionType : public Type
{
public:
    FunctionType(Type* returnType, std::vector<Type*> argumentTypes);

    Type* getReturnType() const;
    const std::vector<Type*>& getArgumentTypes() const;

    virtual int getSize() const override;
    virtual vipir::Type* getVipirType() const override;
    virtual CastLevel castTo(Type* destType) const override;

    bool isFunctionType() const override;

    static FunctionType* Create(Type* returnType, std::vector<Type*> argumentTypes);
    static void Reset();

private:
    std::vector<Type*> mArgumentTypes;
    Type* mReturnType;
};

#endif // BASILISK_FRAMEWORK_TYPE_FUNCTION_TYPE_H