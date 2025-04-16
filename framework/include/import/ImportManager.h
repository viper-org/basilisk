// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_SYMBOL_IMPORT_MANAGER_H
#define BASILISK_FRAMEWORK_SYMBOL_IMPORT_MANAGER_H 1

#include "scope/Scope.h"

#include "parser/ast/ASTNode.h"

#include <filesystem>
#include <string>
#include <vector>

struct Export;

struct Import
{
    std::filesystem::path from;
    std::filesystem::path to;
};

class ImportManager
{
public:
    ImportManager();

    std::vector<Export> getExports();
    std::vector<std::string> getPendingTypeNames();
    void clearExports();
    void addPendingType(std::string name);
    bool wasExportedTo(std::string root, std::vector<Import>& imports, Export& exp);

    void collectAllImports(std::filesystem::path path, std::filesystem::path relativeTo, std::vector<Import>& imports);
    std::vector<parser::ASTNodePtr> resolveImports(std::filesystem::path path, std::filesystem::path relativeTo, Scope* scope, bool exported);

    void reportUnknownTypeErrors();

    void seizeScope(ScopePtr scope);

private:
    std::vector<std::string> mSearchPaths;
    std::vector<std::string> mImportedFiles;
    std::vector<ScopePtr> mScopes;

    std::vector<Export> mExports;
    std::vector<std::string> mPendingTypeNames;
};

struct Export
{
    std::string exportedFrom;
    Symbol* symbol;
    std::string exportedTo;
};

#endif // BASILISK_FRAMEWORK_SYMBOL_IMPORT_MANAGER_H