// Copyright 2025 solar-mist

#include "parser/ast/statement/CompoundStatement.h"

#include <vipir/IR/Instruction/RetInst.h>

#include <cmath>

namespace parser
{
    CompoundStatement::CompoundStatement(ScopePtr ownScope, std::vector<ASTNodePtr> body, SourcePair source)
        : ASTNode(ownScope->parent, source)
        , mBody(std::move(body))
        , mOwnScope(std::move(ownScope))
    {
    }

    vipir::Value* CompoundStatement::codegen(vipir::IRBuilder& builder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        for (auto& node : mBody)
        {
            node->codegen(builder, module, diag);
        }

        return nullptr;
    }

    void CompoundStatement::typeCheck(diagnostic::Diagnostics& diag, bool& exit)
    {
        for (auto& node : mBody)
        {
            node->typeCheck(diag, exit);
        }
    }
}