// Copyright 2025 solar-mist

#include "type/PendingType.h"
#include "type/ErrorType.h"

static std::vector<PendingType*> pendings;
static std::vector<std::unique_ptr<ErrorType> > incompletes;

PendingType::PendingType(SourcePair source, std::string name, std::vector<StructType::Field> fields)
    : Type("struct " + name)
    , mName(std::move(name))
    , mSource(source)
    , mFields(std::move(fields))
{
}

SourcePair PendingType::getSource()
{
    return mSource;
}

int PendingType::getSize() const
{
    int ret = 0;
    for (auto& field : mFields)
    {
        ret += field.type->getSize();
    }
    return ret;
}

vipir::Type* PendingType::getVipirType() const
{
    return mImpl->getVipirType();
}

Type::CastLevel PendingType::castTo(Type* type) const
{
    return mImpl->castTo(type);
}

bool PendingType::isStructType() const
{
    return mImpl->isStructType();
}

void PendingType::initComplete()
{
    mImpl = StructType::Create(mName, mFields, mSource.start.line, mSource.start.col);
    std::erase(pendings, this);
}

void PendingType::initIncomplete()
{
    incompletes.push_back(std::make_unique<ErrorType>(getSize()));
    mImpl = incompletes.back().get();
    std::erase(pendings, this);
}

void PendingType::set(std::vector<StructType::Field> fields)
{
    mImpl = nullptr;
    mFields = std::move(fields);
    pendings.push_back(this);
}

StructType* PendingType::get()
{
    return dynamic_cast<StructType*>(mImpl);
}

PendingType* PendingType::Create(SourcePair source, std::string name, std::vector<StructType::Field> fields)
{
    void AddType(std::string name, std::unique_ptr<Type> type);
    auto typePtr = std::make_unique<PendingType>(std::move(source), name, std::move(fields));

    auto type = typePtr.get();
    pendings.push_back(type);

    AddType(name, std::move(typePtr));

    return type;
}

std::vector<PendingType*>& PendingType::GetPending()
{
    return pendings;
}