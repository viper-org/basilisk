// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_SCOPE_SCOPE_H
#define BASILISK_FRAMEWORK_SCOPE_SCOPE_H 1

#include "type/Type.h"

#include <vipir/IR/Value.h>
#include <vipir/IR/BasicBlock.h>

#include <memory>
#include <string>
#include <vector>

struct Scope;
struct Symbol;

struct Symbol
{
    Symbol(std::string name, Type* type);

    vipir::Value* getLatestValue(vipir::BasicBlock* basicBlock = nullptr);

    std::string name;
    Type* type;
    std::vector<std::pair<vipir::BasicBlock*, vipir::Value*> > values;
};
using SymbolPtr = std::unique_ptr<Symbol>;

struct Scope
{
    Scope(Scope* parent);

    static Scope* GetGlobalScope();

    Symbol* getSymbol(unsigned long id);
    Symbol* resolveSymbol(std::string name);

    Scope* parent;

    Type* currentReturnType;

    std::vector<SymbolPtr> symbols;

    std::vector<Scope*> children;
};
using ScopePtr = std::unique_ptr<Scope>;

#endif // BASILISK_FRAMEWORK_SCOPE_SCOPE_H