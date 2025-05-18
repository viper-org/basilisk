// Copyright 2025 solar-mist

#include "parser/ast/global/ImportStatement.h"

#include <vipir/IR/GlobalVar.h>
#include <vipir/Module.h>
#include <vipir/IR/Instruction/AllocaInst.h>
#include <vipir/IR/Instruction/AddrInst.h>

namespace parser
{
    ImportStatement::ImportStatement(Scope* scope, std::vector<std::string> module, SourcePair source)
        : ASTNode(scope, source, nullptr)
        , mModule(std::move(module))
    {
    }

    vipir::Value* ImportStatement::codegen(vipir::IRBuilder& builder, vipir::DIBuilder& diBuilder, vipir::Module& module, diagnostic::Diagnostics& diag)
    {
        return nullptr;
    }

    void ImportStatement::typeCheck(diagnostic::Diagnostics& diag, bool& exit)
    {
    }

    std::vector<std::string> ImportStatement::getModule()
    {
        return mModule;
    }
}