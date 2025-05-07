// Copyright 2024 solar-mist

#include "parser/ast/expression/StringLiteral.h"

#include "type/SliceType.h"

#include <vipir/IR/Constant/ConstantInt.h>
#include <vipir/IR/Constant/ConstantStruct.h>

#include <vipir/IR/GlobalString.h>

#include <vipir/IR/Instruction/AddrInst.h>

namespace parser
{
    StringLiteral::StringLiteral(Scope* scope, std::string value, SourcePair source)
        : ASTNode(scope, source, SliceType::Get(Type::Get("i8")))
        , mValue(std::move(value))
    {
    }

    vipir::Value* StringLiteral::codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        vipir::GlobalString* string = vipir::GlobalString::Create(module, mValue);

        auto addr = builder.CreateAddrOf(string);
        auto len = vipir::ConstantInt::Get(module, mValue.size(), vipir::Type::GetIntegerType(64));
        return builder.CreateConstantStruct(mType->getVipirType(), { addr, len });
    }
    
    void StringLiteral::typeCheck(diagnostic::Diagnostics&, bool&)
    {
    }
}