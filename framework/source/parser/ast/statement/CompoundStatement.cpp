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

    vipir::Value* CompoundStatement::codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        for (auto& node : mBody)
        {
            node->dcodegen(builder, diBuilder, module, diag);
        }

        return nullptr;
    }

    std::vector<ASTNode*> CompoundStatement::getChildren()
    {
        std::vector<ASTNode*> children;
        for (auto& node : mBody)
        {
            children.push_back(node.get());
        }
        return children;
    }

    void CompoundStatement::typeCheck(diagnostic::Diagnostics& diag, bool& exit)
    {
        for (auto& node : mBody)
        {
            node->typeCheck(diag, exit);
        }
    }
}