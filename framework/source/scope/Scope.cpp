// Copyright 2025 solar-mist

#include "scope/Scope.h"

#include <algorithm>

Symbol::Symbol(std::string name, Type* type)
    : name(std::move(name))
    , type(type)
{
}

vipir::Value* Symbol::getLatestValue(vipir::BasicBlock* basicBlock)
{
    if (!basicBlock)
    {
        return values.back().second;
    }
    // Avoid searching the same basicblock twice in case of recursion
    if (std::find(searched.begin(), searched.end(), basicBlock) != searched.end()) return nullptr;
    searched.push_back(basicBlock);

    auto it = std::find_if(values.rbegin(), values.rend(), [basicBlock](const auto& value) {
        return value.first == basicBlock;
    });
    if (it != values.rend())
    {
        searched.clear();
        return it->second;
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


Scope::Scope(Scope* parent)
    : parent(parent)
{
}

Scope* Scope::GetGlobalScope()
{
    static Scope globalScope(nullptr);
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