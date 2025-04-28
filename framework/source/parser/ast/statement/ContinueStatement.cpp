// Copyright 2025 solar-mist

#include "parser/ast/statement/ContinueStatement.h"

#include <vipir/IR/Instruction/BranchInst.h>

#include <cmath>

namespace parser
{
    ContinueStatement::ContinueStatement(Scope* scope, SourcePair source)
        : ASTNode(scope, source)
    {
    }

    vipir::Value* ContinueStatement::codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        auto bb = mScope->getContinueTo();
        if (!bb)
        {
            diag.reportCompilerError(
                mSource.start,
                mSource.end,
                "continue statement not within a loop"
            );
            return nullptr;
        }
        builder.CreateBr(bb);
        return nullptr;
    }

    void ContinueStatement::typeCheck(diagnostic::Diagnostics& diag, bool& exit)
    {
    }
}