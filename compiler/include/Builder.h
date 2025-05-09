// Copyright 2025 solar-mist

#ifndef BASILISK_COMPILER_BUILDER_H
#define BASILISK_COMPILER_BUILDER_H 1

#include "Options.h"

#include "diagnostic/Diagnostic.h"

#include "lexer/Token.h"

#include "parser/ast/ASTNode.h"

#include "vtoml/Value.h"

#include <filesystem>

struct CompileUnit
{
    std::filesystem::path path;
    std::string text;

    std::vector<lexer::Token> tokens;
    std::string moduleName;
    std::vector<parser::ASTNodePtr> ast;

    ScopePtr globalScope;

    vipir::Module module{"error"};
};

class Builder
{
public:
    Builder(std::vector<Option> options, diagnostic::Diagnostics& diag);

    void build();

private:
    std::vector<Option> mOptions;
    std::unordered_map<std::string, toml::ValuePtr> mConfig;

    diagnostic::Diagnostics& mDiag;

    vipir::DIBuilder mDiBuilder;
    
    std::unordered_map<std::filesystem::path, std::vector<lexer::Token> > mTokens;
    std::unordered_map<std::string, std::vector<std::filesystem::path> > mModules;
    std::unordered_map<std::filesystem::path, CompileUnit> mCUs;
    std::vector<std::filesystem::path> mObjects;

    void parseConfig(std::filesystem::path configFilePath);

    void compileObjects(std::filesystem::path projectDir);
    void linkExecutable(std::filesystem::path projectDir);
    void linkStaticLibrary(std::filesystem::path projectDir);

    void lexOne(std::filesystem::path inputFilePath);
    void parseModule(std::filesystem::path inputFilePath);
    void parseOne(std::filesystem::path inputFilePath);
    void doImports(std::filesystem::path inputFilePath);
    void compileObject(std::filesystem::path inputFilePath, std::filesystem::path outputFilePath);
};

#endif // BASILISK_COMPILER_BUILDER_H