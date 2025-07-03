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

    virtual int getSize() const override;
    virtual vipir::Type* getVipirType() const override;
    virtual CastLevel castTo(Type* destType) const override;
    virtual std::string getSymbolID(Type* thisType) const override;

    SourcePair getSource();

    bool isStructType() const override;

    void initComplete();
    void initIncomplete();
    void set(std::vector<StructType::Field> fields);

    StructType* get();

    static PendingType* Create(SourcePair source, std::string name, std::vector<StructType::Field> fields);
    static std::vector<PendingType*>& GetPending();

private:
    std::string mName;
    SourcePair mSource;
    Type* mImpl;
    std::vector<StructType::Field> mFields;
};

#endif // BASILISK_FRAMEWORK_TYPE_PENDING_TYPE_H