// Copyright 2025 solar-mist

#include "scope/Scope.h"

#include <algorithm>

Symbol::Symbol(std::string name, Type* type)
    : name(std::move(name))
    , type(type)
{
}

SymbolValue* Symbol::getLatestValue(vipir::BasicBlock* basicBlock)
{
    if (!basicBlock)
    {
        return &values.back();
    }
    // Avoid searching the same basicblock twice in case of recursion
    if (std::find(searched.begin(), searched.end(), basicBlock) != searched.end()) return nullptr;
    searched.push_back(basicBlock);

    auto it = std::find_if(values.rbegin(), values.rend(), [basicBlock](const auto& value) {
        return value.bb == basicBlock;
    });
    if (it != values.rend())
    {
        searched.clear();
        return &*it;
    }
    
    for (auto predecessor : basicBlock->predecessors())
    {
        if (auto value = getLatestValue(predecessor))
        {
            searched.clear();
            return value;
        }
    }

    return nullptr;
}

SymbolValue* Symbol::getLatestValueX(vipir::BasicBlock* basicBlock)
{
    auto it = std::find_if(values.rbegin(), values.rend(), [basicBlock](const auto& value) {
        return value.bb == basicBlock;
    });
    if (it != values.rend())
    {
        return &*it;
    }

    if (basicBlock->predecessors().size() == 1)
    {
        if (auto value = getLatestValueX(basicBlock->predecessors()[0]))
        {
            return value;
        }
    }

    return nullptr;
}


Scope::Scope(Scope* parent)
    : parent(parent)
{
    if (parent) parent->children.push_back(this);
}

static Scope globalScope(nullptr);
void Scope::ResetGlobalScope()
{
    globalScope = Scope(nullptr);
}

Scope* Scope::GetGlobalScope()
{
    return &globalScope;
}

Symbol* Scope::resolveSymbol(std::string name)
{
    Scope* current = this;
    while (current)
    {
        auto it = std::find_if(current->symbols.begin(), current->symbols.end(), [&name](const auto& symbol){
            return symbol->name == name;
        });

        if (it != current->symbols.end()) return it->get();
        current = current->parent;
    }

    return nullptr;
}

Type* Scope::getCurrentReturnType()
{
    Scope* current = this;
    while (current)
    {
        if (current->currentReturnType) return current->currentReturnType;
        current = current->parent;
    }

    return nullptr;
}

vipir::BasicBlock* Scope::getContinueTo(std::string label)
{
    Scope* current = this;
    while (current)
    {
        if (current->loopContext.continueTo)
        {
            if (label.empty() || current->loopContext.label == label)
            {
                return current->loopContext.continueTo;
            }
        }
        current = current->parent;
    }

    return nullptr;
}

vipir::BasicBlock* Scope::getBreakTo(std::string label)
{
    Scope* current = this;
    while (current)
    {
        if (current->loopContext.breakTo)
        {
            if (label.empty() || current->loopContext.label == label)
            {
                return current->loopContext.breakTo;
            }
        }
        current = current->parent;
    }

    return nullptr;
}