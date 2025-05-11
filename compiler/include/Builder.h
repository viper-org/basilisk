// Copyright 2025 solar-mist

#ifndef BASILISK_COMPILER_BUILDER_H
#define BASILISK_COMPILER_BUILDER_H 1

#include "diagnostic/Diagnostic.h"

#include "lexer/Token.h"

#include "parser/ast/ASTNode.h"

#include <vtoml/Value.h>

#include <vipir/Module.h>

#include <filesystem>
#include <vector>

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

// For writing
struct ModuleEntry
{
    std::vector<parser::ASTNode*> symbols;
    uint32_t headerOffset;
};
// For reading
struct ModuleDetails
{
    std::string name;

    uint32_t funcOffset;
    uint32_t funcLength;
    uint32_t structOffset;
    uint32_t structLength;
};

class BSLib
{
public:
    void write(std::uint8_t  data, std::uint64_t offset = -1, bool overwrite = false);
    void write(std::uint16_t data, std::uint64_t offset = -1, bool overwrite = false);
    void write(std::uint32_t data, std::uint64_t offset = -1, bool overwrite = false);
    void write(std::uint64_t data, std::uint64_t offset = -1, bool overwrite = false);
    void write(std::string_view data);

    void write(const char* data, size_t size, std::uint64_t offset, bool overwrite);

    std::vector<char>& getBuffer();

private:
    std::vector<char> mBuffer;
};

class Builder
{
public:
    Builder(diagnostic::Diagnostics& diag);

    void build();
    std::string getOutputFile();

private:
    std::unordered_map<std::string, toml::ValuePtr> mConfig;
    Scope mLibraryScope { nullptr };

    diagnostic::Diagnostics& mDiag;

    vipir::DIBuilder mDiBuilder;
    
    std::unordered_map<std::filesystem::path, std::vector<lexer::Token> > mTokens;
    std::unordered_map<std::string, std::vector<std::filesystem::path> > mModules;
    std::unordered_map<std::string, std::vector<parser::ASTNodePtr> > mImportedModules;
    std::vector<std::string> mArchives;
    std::unordered_map<std::filesystem::path, CompileUnit> mCUs;
    std::vector<std::filesystem::path> mObjects;
    std::unordered_map<std::string, ModuleEntry> mSymbolEntries;

    void parseConfig(std::filesystem::path configFilePath);

    void collectLibraries(std::filesystem::path projectDir);
    void parseLibrary(std::string library, std::filesystem::path projectDir);

    void compileObjects(std::filesystem::path projectDir);
    void generateSymbolFile(std::filesystem::path projectDir);
    void linkExecutable(std::filesystem::path projectDir);
    void linkStaticLibrary(std::filesystem::path projectDir);

    void lexOne(std::filesystem::path inputFilePath);
    void parseModule(std::filesystem::path inputFilePath);
    void parseOne(std::filesystem::path inputFilePath);
    void doImports(std::filesystem::path inputFilePath);
    void compileObject(std::filesystem::path inputFilePath, std::filesystem::path outputFilePath);
};

#endif // BASILISK_COMPILER_BUILDER_H