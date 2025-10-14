// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_TYPE_PENDING_TYPE_H
#define BASILISK_FRAMEWORK_TYPE_PENDING_TYPE_H 1

#include "type/Type.h"
#include "type/StructType.h"

#include "debug/SourcePair.h"

class PendingType : public Type
{
public:
    PendingType(SourcePair source, std::string name, std::vector<StructType::Field> fields);

    virtual bool isIntegerType()  const override { return mImpl->isIntegerType();  }
    virtual bool isVoidType()     const override { return mImpl->isVoidType();     }
    virtual bool isFunctionType() const override { return mImpl->isFunctionType(); }
    virtual bool isBooleanType()  const override { return mImpl->isBooleanType();  }
    virtual bool isPointerType()  const override { return mImpl->isPointerType();  }
    virtual bool isSliceType()    const override { return mImpl->isSliceType();    }
    virtual bool isArrayType()    const override { return mImpl->isArrayType();    }
    virtual bool isStructType()   const override { return mImpl->isStructType();   }

    virtual int getSize() const override;
    virtual vipir::Type* getVipirType() const override;
    virtual CastLevel castTo(Type* destType) const override;
    virtual std::string getSymbolID(Type* thisType) const override;

    SourcePair getSource();

    void initComplete();
    void initIncomplete();
    void initAlias(Type* aliasOf);
    void set(std::vector<StructType::Field> fields);

    StructType* get();
    Type* impl();

    static PendingType* Create(SourcePair source, std::string name, std::vector<StructType::Field> fields);
    static std::vector<PendingType*>& GetPending();

private:
    std::string mName;
    SourcePair mSource;
    Type* mImpl;
    std::vector<StructType::Field> mFields;
};

#endif // BASILISK_FRAMEWORK_TYPE_PENDING_TYPE_H