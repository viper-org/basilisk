// Copyright 2025 solar-mist

#include "parser/ast/expression/NullptrLiteral.h"

#include <type/PointerType.h>

#include <vipir/IR/Constant/ConstantNullPtr.h>

#include <cmath>

namespace parser
{
    NullptrLiteral::NullptrLiteral(Scope* scope, SourcePair source)
        : ASTNode(scope, source, PointerType::Get(Type::Get("void")))
    {
    }

    vipir::Value* NullptrLiteral::codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        return vipir::ConstantNullPtr::Get(module, mType->getVipirType());
    }

    void NullptrLiteral::typeCheck(diagnostic::Diagnostics& diag, bool& exit)
    {
    }

    bool NullptrLiteral::triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType)
    {
        if (destType->isPointerType())
        {
            mType = destType;
            return true;
        }
        return false;
    }
}