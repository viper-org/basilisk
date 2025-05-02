// Copyright 2025 solar-mist

#include "parser/ast/statement/BreakStatement.h"

#include <vipir/IR/Instruction/BranchInst.h>

#include <cmath>

namespace parser
{
    BreakStatement::BreakStatement(Scope* scope, std::string label, SourcePair source)
        : ASTNode(scope, source)
        , mLabel(std::move(label))
    {
    }

    vipir::Value* BreakStatement::codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        auto bb = mScope->getBreakTo(mLabel);
        if (!bb)
        {
            diag.reportCompilerError(
                mSource.start,
                mSource.end,
                "break statement not within a loop"
            );
            return nullptr;
        }
        builder.CreateBr(bb);
        return nullptr;
    }

    void BreakStatement::typeCheck(diagnostic::Diagnostics& diag, bool& exit)
    {
    }
}