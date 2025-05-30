// Copyright 2025 solar-mist

#include "parser/ast/ASTNode.h"

#include "parser/ast/expression/CastExpression.h"

namespace parser
{
    bool ASTNode::canImplicitCast(diagnostic::Diagnostics& diag, Type* destType)
    {
        if (triviallyImplicitCast(diag, destType))
        {
            return true;
        }

        auto cast = mType->castTo(destType);
        if (cast == Type::CastLevel::Implicit || cast == Type::CastLevel::ImplicitWarning)
        {
            if (cast == Type::CastLevel::ImplicitWarning)
            {
                diag.compilerWarning(
                    "implicit",
                    mSource.start,
                    mSource.end,
                    mType->getImplicitCastWarning(destType)
                );
            }
            return true;
        }
        return false;
    }

    ASTNodePtr ASTNode::Cast(ASTNodePtr& node, Type* destType)
    {
        auto source = node->mSource;
        if (node->mType == destType)
        {
            return std::move(node);
        }
        return std::make_unique<CastExpression>(node->mScope, std::move(node), destType, std::move(source));
    }
}