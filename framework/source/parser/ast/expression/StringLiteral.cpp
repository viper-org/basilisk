// Copyright 2024 solar-mist

#include "parser/ast/expression/StringLiteral.h"

#include "type/SliceType.h"
#include "type/PointerType.h"

#include <vipir/IR/Constant/ConstantInt.h>
#include <vipir/IR/Constant/ConstantStruct.h>

#include <vipir/IR/GlobalString.h>

#include <vipir/IR/Instruction/AddrInst.h>

namespace parser
{
    StringLiteral::StringLiteral(Scope* scope, std::string value, bool cString, SourcePair source)
        : ASTNode(scope, source, SliceType::Get(Type::Get("i8")))
        , mValue(std::move(value))
        , mCString(cString)
    {
        if (mCString)
        {
            mType = PointerType::Get(Type::Get("i8"));
        }
    }

    vipir::Value* StringLiteral::codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        auto length = mValue.size();
        if (mCString) mValue += '\0';
        vipir::GlobalString* string = vipir::GlobalString::Create(module, mValue);

        auto addr = builder.CreateAddrOf(string);
        if (mCString) return addr;

        auto len = vipir::ConstantInt::Get(module, length, vipir::Type::GetIntegerType(64));
        return builder.CreateConstantStruct(mType->getVipirType(), { addr, len });
    }
    
    void StringLiteral::typeCheck(diagnostic::Diagnostics&, bool&)
    {
    }
}