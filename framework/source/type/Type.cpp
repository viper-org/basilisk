// Copyright 2025 solar-mist

#include "type/Type.h"
#include "type/IntegerType.h"
#include "type/VoidType.h"
#include "type/BooleanType.h"
#include "type/PointerType.h"
#include "type/StructType.h"
#include "type/ErrorType.h"
#include "type/PendingType.h"

#include <dwarf.h>
#include <unordered_map>

std::unordered_map<std::string, std::unique_ptr<Type>> types;

void AddType(std::string name, std::unique_ptr<Type> type)
{
    types[name] = std::move(type);
}

vipir::DIBuilder* builder;
vipir::DIBuilder* Type::GetDIBuilder()
{
    return builder;
}

void Type::Init(vipir::DIBuilder* diBuilder)
{
    types["i8"]  = std::make_unique<IntegerType>(8, true);
    types["i16"] = std::make_unique<IntegerType>(16, true);
    types["i32"] = std::make_unique<IntegerType>(32, true);
    types["i64"] = std::make_unique<IntegerType>(64, true);
    types["u8"]  = std::make_unique<IntegerType>(8, false);
    types["u16"] = std::make_unique<IntegerType>(16, false);
    types["u32"] = std::make_unique<IntegerType>(32, false);
    types["u64"] = std::make_unique<IntegerType>(64, false);

    types["void"] = std::make_unique<VoidType>();
    types["bool"] = std::make_unique<BooleanType>();

    types["error-type"] = std::make_unique<ErrorType>();


    types["i8"]->setDiType(diBuilder->createBasicType("i8", types["i8"]->getVipirType(), DW_ATE_signed_char));
    types["i16"]->setDiType(diBuilder->createBasicType("i16", types["i16"]->getVipirType(), DW_ATE_signed));
    types["i32"]->setDiType(diBuilder->createBasicType("i32", types["i32"]->getVipirType(), DW_ATE_signed));
    types["i64"]->setDiType(diBuilder->createBasicType("i64", types["i64"]->getVipirType(), DW_ATE_signed));

    types["u8"]->setDiType(diBuilder->createBasicType("u8", types["u8"]->getVipirType(), DW_ATE_unsigned_char));
    types["u16"]->setDiType(diBuilder->createBasicType("u16", types["u16"]->getVipirType(), DW_ATE_unsigned));
    types["u32"]->setDiType(diBuilder->createBasicType("u32", types["u32"]->getVipirType(), DW_ATE_unsigned));
    types["u64"]->setDiType(diBuilder->createBasicType("u64", types["u64"]->getVipirType(), DW_ATE_unsigned));

    types["bool"]->setDiType(diBuilder->createBasicType("bool", types["bool"]->getVipirType(), DW_ATE_boolean));

    types["void"]->setDiType(diBuilder->createBasicType("void", types["void"]->getVipirType(), DW_ATE_void));

    builder = diBuilder;
}

bool Type::Exists(const std::string& name)
{
    auto type = types.find(name);
    return type != types.end();
}

Type* Type::Get(const std::string& name)
{
    auto type = types.find(name);
    if (type != types.end()) return type->second.get();

    return nullptr;
}

void Type::FinalizeDITypes()
{
    for (auto& [_, type] : types)
    {
        if (auto pending = dynamic_cast<PendingType*>(type.get()))
        {
            pending->mDiType = pending->get()->mDiType;
        }
    }
    PointerType::SetDITypes();
    StructType::SetDITypes();
}