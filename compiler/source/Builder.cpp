// Copyright 2025 solar-mist

#include "Builder.h"
#include "Linker.h"

#include "lexer/Lexer.h"

#include "parser/Parser.h"

#include "type/Type.h"

#include <vipir/ABI/SysV.h>
#include <vipir/Pass/DefaultPass.h>

#include <filesystem>
#include <fstream>
#include <sstream>

Builder::Builder(std::vector<Option> options, diagnostic::Diagnostics& diag)
    : mOptions(std::move(options))
    , mDiag(diag)
{
}

void Builder::build()
{
    auto inputFiles = Option::GetInputFiles(mOptions);
    std::filesystem::path projectDir;
    if (inputFiles.empty())
    {
        projectDir = std::filesystem::current_path();
    }
    else if (inputFiles.size() == 1)
    {
        projectDir = std::filesystem::path(inputFiles[0]);
    }
    else
    {
        mDiag.fatalError("multiple project directories specified");
        std::exit(1);
    }

    if (!std::filesystem::is_directory(projectDir))
    {
        mDiag.fatalError(std::format("'{}{}{}' is not a valid project directory", fmt::bold, projectDir.string(), fmt::defaults));
        std::exit(1);
    }

    std::filesystem::path config = projectDir / "basilisk.toml";
    if (!std::filesystem::exists(config))
    {
        mDiag.fatalError(std::format("'{}{}{}' is not a valid basilisk project", fmt::bold, projectDir.string(), fmt::defaults));
        std::exit(1);
    }

    std::ifstream configFile(config);
    std::stringstream buffer;
    buffer << configFile.rdbuf();
    std::string configText = std::move(buffer).str();
    configFile.close();

    // TODO: Add module scanning

    compileObjects(projectDir);
    // TODO: Read bsproj file to determine build type
    linkExecutable(projectDir);
}


void Builder::compileObjects(std::filesystem::path projectDir)
{
    std::filesystem::path sourceDir = projectDir / "src";
    std::filesystem::path buildDir = projectDir / "build";
    std::vector<std::pair<std::filesystem::path, std::filesystem::path> > files;

    Type::Init(&mDiBuilder);

    for (std::filesystem::recursive_directory_iterator it(sourceDir); it != std::filesystem::end(it); ++it)
    {
        if (it->is_regular_file() && it->path().extension() == ".bslk")
        {
            auto inputFile = it->path();
            auto inputFileRelativePath = std::filesystem::relative(inputFile, sourceDir);
            auto outputFile = buildDir / "objects" / inputFileRelativePath;
            outputFile.replace_extension(".o");
            std::filesystem::create_directories(outputFile.parent_path());
            mObjects.push_back(outputFile);
            files.push_back({inputFile, outputFile});

            lexOne(inputFile);
            parseModule(inputFile);
            parseOne(inputFile);
        }
    }
    Type::FinalizeDITypes();

    for (auto& file : files)
    {
        doImports(file.first);
    }

    for (auto& file : files)
    {
        compileObject(file.first, file.second);
    }
}

void Builder::linkExecutable(std::filesystem::path projectDir)
{
    auto outputFile = projectDir / "a.out";

    std::vector<Option> linkOptions;
    for (auto objectFile : mObjects)
    {
        linkOptions.push_back({OptionType::InputFile, objectFile.string()});
    }
    linkOptions.push_back({OptionType::OutputFile, outputFile});
    Linker linker(linkOptions, mDiag);
    linker.linkExecutable();
}


void Builder::lexOne(std::filesystem::path inputFilePath)
{
    std::ifstream inputFile(inputFilePath);
    std::stringstream buffer;
    buffer << inputFile.rdbuf();
    mCUs[inputFilePath].text = std::move(buffer).str();
    inputFile.close();

    lexer::Lexer lexer(mCUs[inputFilePath].text, inputFilePath.string());
    auto tokens = lexer.lex();
    mCUs[inputFilePath].tokens = tokens;
}

void Builder::parseModule(std::filesystem::path inputFilePath)
{
    auto& tokens = mCUs[inputFilePath].tokens;
    if (tokens[0].getTokenType() == lexer::TokenType::ModuleKeyword)
    {
        if (tokens[1].getTokenType() == lexer::TokenType::Identifier)
        {
            std::string moduleName(tokens[1].getText());
            mModules[moduleName].push_back(inputFilePath.string());
            mCUs[inputFilePath].moduleName = std::move(moduleName);
        }
        else
        {
            mDiag.reportCompilerError(
                tokens[1].getStartLocation(),
                tokens[1].getEndLocation(),
                std::format("expected module name after '{}module{}' keyword", fmt::bold, fmt::defaults));
            std::exit(1);
        }
    }
    else
    {
        mDiag.reportCompilerError(
            tokens[0].getStartLocation(),
            tokens[0].getEndLocation(),
            std::format("expected '{}module{}' keyword", fmt::bold, fmt::defaults));
        std::exit(1);
    }
}

void Builder::parseOne(std::filesystem::path inputFilePath)
{
    ImportManager importManager;

    mCUs[inputFilePath].globalScope = std::make_unique<Scope>(nullptr);
    auto tokens = mCUs[inputFilePath].tokens;

    parser::Parser parser(tokens, mDiag, importManager, mCUs[inputFilePath].globalScope.get(), false);

    auto ast = parser.parse();
    mCUs[inputFilePath].ast = std::move(ast);
    importManager.reportUnknownTypeErrors();
}

void Builder::doImports(std::filesystem::path inputFilePath)
{
    auto& ast = mCUs[inputFilePath].ast;

    // TODO: Get imports other than current module

    for (auto& file : mModules[mCUs[inputFilePath].moduleName])
    {
        if (file != inputFilePath)
        {
            auto& ast = mCUs[file].ast;

            for (auto& node : ast)
            {
                if (auto cloned = node->cloneExternal(mCUs[inputFilePath].globalScope.get()))
                {
                    mCUs[inputFilePath].ast.insert(mCUs[inputFilePath].ast.begin(), std::move(cloned));
                }
            }
        }
    }
}

void Builder::compileObject(std::filesystem::path inputFilePath, std::filesystem::path outputFilePath)
{
    auto& ast = mCUs[inputFilePath].ast;
    auto& module = mCUs[inputFilePath].module;

    mCUs[inputFilePath].module = vipir::Module(inputFilePath.string());

    vipir::DIBuilder diBuilder;
    diBuilder.setProducer("basilisk");
    diBuilder.setDirectory(inputFilePath.parent_path().string());
    diBuilder.setFilename(inputFilePath.filename().string());
    diBuilder.borrowTypes(&mDiBuilder);

    mDiag.setText(mCUs[inputFilePath].text);

    auto& tokens = mTokens[inputFilePath];

    module.setABI<vipir::abi::SysV>();
    Option::ParseOptimizingFlags(mOptions, module, mDiag);
    // Always do constant folding
    if (module.getPassManager().findPass(vipir::PassType::ConstantFolding) == -1)
        module.getPassManager().insertBefore(vipir::PassType::LIREmission, std::make_unique<vipir::ConstantFoldingPass>());

    vipir::IRBuilder builder;

    bool exit = false;
    for (auto& node : ast)
    {
        node->typeCheck(mDiag, exit);
    }
    if (exit) std::exit(1);

    for (auto& node : ast)
    {
        node->codegen(builder, diBuilder, module, mDiag);
    }
    
    std::function<void(Scope*)> checkOne;
    checkOne = [&checkOne](Scope* scope) {
        for (auto& symbol : scope->symbols)
        {
            if (symbol->diVariable)
            {
                for (auto& value : symbol->values)
                {
                    if (value.pointer)
                    {
                        symbol->diVariable->addPointer(value.pointer, value.start, value.end);
                    }
                    else
                    {
                        symbol->diVariable->addValue(value.value, value.start, value.end);
                    }
                }
            }
        }
        for (auto child : scope->children)
        {
            checkOne(child);
        }
    };
    checkOne(mCUs[inputFilePath].globalScope.get());

    std::ofstream outputFile(outputFilePath);
    module.setOutputFormat(vipir::OutputFormat::ELF);
    module.emit(outputFile);
}