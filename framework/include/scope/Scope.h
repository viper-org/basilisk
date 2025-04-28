// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_SCOPE_SCOPE_H
#define BASILISK_FRAMEWORK_SCOPE_SCOPE_H 1

#include "type/Type.h"

#include <vipir/IR/Value.h>
#include <vipir/IR/BasicBlock.h>
#include <vipir/DI/DIVariable.h>

#include <memory>
#include <string>
#include <vector>

struct Scope;
struct Symbol;

struct SymbolValue
{
    vipir::BasicBlock* bb;
    vipir::Value* value;
    vipir::QueryAddress* start;
    vipir::QueryAddress* end;

    /*
     * Only used in cases such as these:
     *  y = &x;
     */
    vipir::DIVariable* pointer;
};

struct Symbol
{
    Symbol(std::string name, Type* type);

    SymbolValue* getLatestValue(vipir::BasicBlock* basicBlock = nullptr);
    // Only checks the specific basicblock provided
    SymbolValue* getLatestValueX(vipir::BasicBlock* basicBlock);

    std::string name;
    Type* type;
    std::vector<SymbolValue> values;
    vipir::DIVariable* diVariable { nullptr };
    std::vector<vipir::BasicBlock*> searched;
    
    bool exported { false };
    bool removed { false };
};
using SymbolPtr = std::unique_ptr<Symbol>;

struct Scope
{
    Scope(Scope* parent);

    static Scope* GetGlobalScope();

    Symbol* getSymbol(unsigned long id);
    Symbol* resolveSymbol(std::string name);

    Type* getCurrentReturnType();
    vipir::BasicBlock* getContinueTo();
    vipir::BasicBlock* getBreakTo();

    Scope* parent;

    Type* currentReturnType{ nullptr };
    vipir::BasicBlock* continueTo{ nullptr };
    vipir::BasicBlock* breakTo{ nullptr };

    std::vector<SymbolPtr> symbols;

    std::vector<Scope*> children;
};
using ScopePtr = std::unique_ptr<Scope>;

#endif // BASILISK_FRAMEWORK_SCOPE_SCOPE_H