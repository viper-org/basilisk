// Copyright 2025 solar-mist

#include "Builder.h"
#include "Linker.h"

#include "lexer/Lexer.h"

#include "parser/Parser.h"

#include "type/Type.h"
#include "type/PointerType.h"
#include "type/SliceType.h"
#include "type/StructType.h"

#include <vtoml/parser/Parser.h>

#include <vipir/ABI/SysV.h>
#include <vipir/Pass/DefaultPass.h>

#include <vasm/lexer/Lexer.h>
#include <vasm/lexer/Token.h>

#include <vasm/parser/Parser.h>

#include <vasm/error/ErrorReporter.h>

#include <vasm/codegen/Elf.h>

#include <filesystem>
#include <fstream>
#include <sstream>

void BSLib::write(std::uint8_t data, std::uint64_t offset, bool overwrite)
{
    write(reinterpret_cast<const char*>(&data), sizeof(data), offset, overwrite);
}

void BSLib::write(std::uint16_t data, std::uint64_t offset, bool overwrite)
{
    write(reinterpret_cast<const char*>(&data), sizeof(data), offset, overwrite);
}

void BSLib::write(std::uint32_t data, std::uint64_t offset, bool overwrite)
{
    write(reinterpret_cast<const char*>(&data), sizeof(data), offset, overwrite);
}

void BSLib::write(std::uint64_t data, std::uint64_t offset, bool overwrite)
{
    write(reinterpret_cast<const char*>(&data), sizeof(data), offset, overwrite);
}

void BSLib::write(std::string_view data)
{
    write(data.data(), data.size(), -1, false);
}

void BSLib::write(const char* data, size_t size, std::uint64_t offset, bool overwrite)
{
    for (; size; --size) {
        if (offset != -1)
        {
            if (overwrite)
                mBuffer[offset] = *data++;
            else
                mBuffer.insert(mBuffer.begin() + offset, *data++);
            ++offset;
        }
        else
            mBuffer.push_back(*data++);
    }
}

std::vector<char>& BSLib::getBuffer()
{
    return mBuffer;
}


Builder::Builder(diagnostic::Diagnostics& diag)
    : mDiag(diag)
{
}

void Builder::build()
{
    std::filesystem::path projectDir = std::filesystem::current_path();

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
    parseConfig(projectDir / "basilisk.toml");

    Type::Init(&mDiBuilder);

    if (mConfig.find("link") != mConfig.end())
        collectLibraries(projectDir);

    std::ifstream configFile(config);
    std::stringstream buffer;
    buffer << configFile.rdbuf();
    std::string configText = std::move(buffer).str();
    configFile.close();

    compileObjects(projectDir);
    if (mConfig["target"]->toString() == "exe")
    {
        linkExecutable(projectDir);
    }
    else if (mConfig["target"]->toString() == "lib")
    {
        linkStaticLibrary(projectDir);
    }
    else
    {
        mDiag.fatalError(std::format("unknown target '{}'", mConfig["target"]->toString()));
        std::exit(1);
    }
}

std::string Builder::getOutputFile()
{
    return mConfig["name"]->toString();
}


void Builder::parseConfig(std::filesystem::path configFilePath)
{
    mConfig = toml::ParseFile(configFilePath);
}

void Builder::collectLibraries(std::filesystem::path projectDir)
{
    auto& libs = mConfig["link"];
    if (!libs->isArray())
    {
        mDiag.fatalError("link option in config must be an array");
        std::exit(1);
    }

    for (auto& lib : libs->toArray())
    {
        if (!lib->isString())
        {
            mDiag.fatalError("link option in config must be an array of strings");
            std::exit(1);
        }
        parseLibrary(lib->toString(), projectDir);
    }
}

void Builder::parseLibrary(std::string lib, std::filesystem::path projectDir)
{
    auto libDir = projectDir / "libs";

    auto libPath = libDir / lib;
    libPath.replace_extension(".bslib");
    if (!std::filesystem::exists(libPath))
    {
        mDiag.fatalError(std::format("library '{}' not found", lib));
        std::exit(1);
    }

    std::ifstream libFile(libPath, std::ios::binary);
    char buffer[4];
    libFile.read(buffer, 4);
    if (std::string(buffer, 4) != "BSLK")
    {
        mDiag.fatalError(std::format("library '{}' has invalid file format", lib));
        std::exit(1);
    }
    uint32_t length;
    libFile.read(reinterpret_cast<char*>(&length), sizeof(length));
    uint32_t moduleCount;
    libFile.read(reinterpret_cast<char*>(&moduleCount), sizeof(moduleCount));
    std::vector<ModuleDetails> modules;
    modules.reserve(moduleCount);
    for (uint32_t i = 0; i < moduleCount; ++i)
    {
        std::string moduleName;
        char c;
        while (libFile.get(c) && c != '\0')
        {
            moduleName += c;
        }
        uint32_t funcOffset, funcLength, structOffset, structLength;
        libFile.read(reinterpret_cast<char*>(&funcOffset), sizeof(funcOffset));
        libFile.read(reinterpret_cast<char*>(&funcLength), sizeof(funcLength));
        libFile.read(reinterpret_cast<char*>(&structOffset), sizeof(structOffset));
        libFile.read(reinterpret_cast<char*>(&structLength), sizeof(structLength));
        modules.push_back({moduleName, funcOffset, funcLength, structOffset, structLength});
    }

    for (auto& module : modules)
    {
        libFile.seekg(module.funcOffset);
        while (libFile.tellg() < module.funcOffset + module.funcLength)
        {
            uint32_t nameSize;
            libFile.read(reinterpret_cast<char*>(&nameSize), sizeof(nameSize));
            std::string name(nameSize, '\0');
            libFile.read(name.data(), nameSize);

            std::function<Type*(uint8_t)> parseType;
            parseType = [this, &libFile, &parseType](uint8_t c) -> Type* {
                switch (c)
                {
                    // Signed integrals
                    case 'c': return Type::Get("i8");
                    case 's': return Type::Get("i16");
                    case 'i': return Type::Get("i32");
                    case 'l': return Type::Get("i64");
                    
                    // Unsigned integrals
                    case 'b': return Type::Get("u8");
                    case 'w': return Type::Get("u16");
                    case 'd': return Type::Get("u32");
                    case 'q': return Type::Get("u64");

                    case 'B': return Type::Get("bool");
                    case 'v': return Type::Get("void");

                    case 'P': return PointerType::Get(parseType(libFile.get()));
                    case 'Z': return SliceType::Get(parseType(libFile.get()));
                    case 'S':
                    {
                        uint32_t length;
                        std::string lengthStr;
                        char c;
                        while (libFile.get(c) && isdigit(c))
                        {
                            lengthStr += c;
                        }
                        length = std::stoul(lengthStr);
                        auto buf = new char[length];
                        libFile.read(buf, length);

                        auto type = StructType::Get(buf);
                        delete[] buf;
                        return type;
                    }

                    default:
                        mDiag.fatalError("library file contains unknown type");
                        std::exit(1);
                }
            };
            std::vector<Type*> argTypes;
            while (true)
            {
                uint8_t c;
                libFile.read(reinterpret_cast<char*>(&c), sizeof(c));
                if (c == 'E')
                    break;
                argTypes.push_back(parseType(c));
            }
            auto retType = parseType(libFile.get());
            auto functionType = FunctionType::Create(retType, argTypes);
            SourcePair source;
            auto funcScope = std::make_unique<Scope>(&mLibraryScope);
            auto func = std::make_unique<parser::Function>(
                true,
                nullptr, // TODO: Get impl type
                name,
                functionType,
                std::vector<parser::FunctionArgument>{},
                std::move(funcScope),
                true,
                std::vector<parser::ASTNodePtr>{},
                source,
                source
            );
            mImportedModules[module.name].push_back(std::move(func));
            auto c = libFile.get(); // null terminator
        }
    }

    libFile.seekg(0, std::ios::end);
    int fileSize = libFile.tellg();
    std::string data;
    data.resize(fileSize - length);
    libFile.seekg(length);
    libFile.read(data.data(), fileSize - length);

    std::filesystem::create_directories(projectDir / "build");
    auto archivePath = projectDir / "build" / lib;
    archivePath.replace_extension(".a");
    std::ofstream archive(archivePath);
    archive.write(data.data(), data.size());
    mArchives.push_back(archivePath.string());
    archive.close();
}

void Builder::compileObjects(std::filesystem::path projectDir)
{
    std::filesystem::path sourceDir = projectDir / "src";
    std::filesystem::path buildDir = projectDir / "build";
    std::vector<std::pair<std::filesystem::path, std::filesystem::path> > files;
    std::vector<std::pair<std::filesystem::path, std::filesystem::path> > vasmFiles;

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
        else if (it->is_regular_file() && it->path().extension() == ".vas")
        {
            auto inputFile = it->path();
            auto inputFileRelativePath = std::filesystem::relative(inputFile, sourceDir);
            auto outputFile = buildDir / "objects" / inputFileRelativePath;
            outputFile.replace_extension(".o");
            std::filesystem::create_directories(outputFile.parent_path());
            mObjects.push_back(outputFile);
            vasmFiles.push_back({inputFile, outputFile});
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

    for (auto& vasmFile : vasmFiles)
    {
        assembleOne(vasmFile.first, vasmFile.second);
    }
}

void Builder::generateSymbolFile(std::filesystem::path projectDir)
{
    BSLib lib;
    lib.write("BSLK"); // Magic identifier
    uint32_t lengthOff = lib.getBuffer().size();
    lib.write((uint32_t)0); // Length of the symbol file
    uint32_t moduleCount = mSymbolEntries.size();
    lib.write(moduleCount); // Number of modules
    for (auto& [moduleName, entry] : mSymbolEntries)
    {
        // Null terminated module name
        lib.write(moduleName);
        lib.write((uint8_t)0);
        
        entry.headerOffset = lib.getBuffer().size();

        lib.write((uint32_t)0); // Offset of functions for this module
        lib.write((uint32_t)0); // Length of functions section for this module
        lib.write((uint32_t)0); // Offset of structs for this module
        lib.write((uint32_t)0); // Length of structs section for this module
        // TODO: Add typedefs, enums etc
    }

    for (auto& [moduleName, entry] : mSymbolEntries)
    {
        uint32_t* header = (uint32_t*)(lib.getBuffer().data() + entry.headerOffset);
        auto funcsStart = lib.getBuffer().size();
        *header = funcsStart;
        for (auto& symbol : entry.symbols)
        {
            if (auto func = dynamic_cast<parser::Function*>(symbol))
            {
                auto name = func->getName();
                uint32_t nameSize = name.size();
                lib.write(nameSize);
                lib.write(name);
                auto functionType = static_cast<FunctionType*>(func->getType());
                for (auto argType : functionType->getArgumentTypes())
                {
                    lib.write(argType->getSymbolID());
                }
                lib.write((uint8_t)'E'); // End of argument types
                lib.write(functionType->getReturnType()->getSymbolID());
                lib.write((uint8_t)0); // End of function
            }
        }
        header = (uint32_t*)(lib.getBuffer().data() + entry.headerOffset);
        *(header + 1) = lib.getBuffer().size() - funcsStart;
    }
    // TODO: Export structs
    *(uint32_t*)(lib.getBuffer().data() + lengthOff) = lib.getBuffer().size();

    std::ofstream symbolFile(projectDir / (mConfig["name"]->toString() + ".bslib"), std::ios::binary);
    symbolFile.write(lib.getBuffer().data(), lib.getBuffer().size());
    symbolFile.close();
}

void Builder::linkExecutable(std::filesystem::path projectDir)
{
    auto outputFile = projectDir / mConfig["name"]->toString();

    std::vector<std::string> inputFiles;
    std::vector<std::string> libraries;
    for (auto objectFile : mObjects)
    {
        inputFiles.push_back(objectFile.string());
    }
    for (auto archive : mArchives)
    {
        libraries.push_back(archive);
    }

    Linker linker(inputFiles, libraries, outputFile, mDiag);
    linker.linkExecutable();
}

void Builder::linkStaticLibrary(std::filesystem::path projectDir)
{
    generateSymbolFile(projectDir);

    auto outputFile = projectDir / "build" / ("lib" + mConfig["name"]->toString());
    outputFile.replace_extension(".a");

    std::vector<std::string> inputFiles;
    for (auto objectFile : mObjects)
    {
        inputFiles.push_back(objectFile.string());
    }

    Linker linker(inputFiles, {}, outputFile, mDiag);
    linker.linkLibrary();

    std::ifstream libFile(outputFile, std::ios::binary);
    std::ofstream bslib(projectDir / (mConfig["name"]->toString() + ".bslib"), std::ios::binary | std::ios::app);
    bslib.seekp(0, std::ios::end);
    bslib << libFile.rdbuf();
}


void Builder::lexOne(std::filesystem::path inputFilePath)
{
    std::ifstream inputFile(inputFilePath);
    std::stringstream buffer;
    buffer << inputFile.rdbuf();
    mCUs[inputFilePath].pathString = inputFilePath.string();
    mCUs[inputFilePath].text = std::move(buffer).str();
    inputFile.close();

    lexer::Lexer lexer(mCUs[inputFilePath].text, mCUs[inputFilePath].pathString);
    auto tokens = lexer.lex();
    mCUs[inputFilePath].tokens = tokens;

    mDiag.addText(mCUs[inputFilePath].pathString, mCUs[inputFilePath].text);
}

void Builder::parseModule(std::filesystem::path inputFilePath)
{
    auto& tokens = mCUs[inputFilePath].tokens;
    if (tokens[0].getTokenType() == lexer::TokenType::ModuleKeyword)
    {
        if (tokens[1].getTokenType() == lexer::TokenType::Identifier)
        {
            int pos = 1;
            std::string moduleName;
            while (tokens[pos].getTokenType() == lexer::TokenType::Identifier)
            {
                moduleName += tokens[pos++].getText();
                if (tokens[pos].getTokenType() == lexer::TokenType::Semicolon) break;
                if (tokens[pos].getTokenType() == basilisk::lexer::TokenType::DoubleColon)
                {
                    ++pos;
                    moduleName += ":";
                }
            }
            mModules[moduleName].push_back(inputFilePath);
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
    mCUs[inputFilePath].globalScope = std::make_unique<Scope>(nullptr);
    auto& tokens = mCUs[inputFilePath].tokens;

    parser::Parser parser(tokens, mDiag, mCUs[inputFilePath].globalScope.get(), false);

    auto ast = parser.parse();
    mCUs[inputFilePath].ast = std::move(ast);

    for (auto& node : mCUs[inputFilePath].ast)
    {
        if (auto func = dynamic_cast<parser::Function*>(node.get()))
        {
            mSymbolEntries[mCUs[inputFilePath].moduleName].symbols.push_back(func);
        }
        else if (auto structDecl = dynamic_cast<parser::StructDeclaration*>(node.get()))
        {
            mSymbolEntries[mCUs[inputFilePath].moduleName].symbols.push_back(structDecl);
        }
    }
}

void Builder::doImports(std::filesystem::path inputFilePath)
{
    auto& ast = mCUs[inputFilePath].ast;

    std::vector<std::string> modules{mCUs[inputFilePath].moduleName};
    for (auto& node : ast)
    {
        if (auto import = dynamic_cast<parser::ImportStatement*>(node.get()))
        {
            std::string module;
            for (auto& name : import->getModule())
            {
                module += name + ":";
            }
            module.pop_back();
            modules.push_back(module);

            if (mModules.find(module) == mModules.end() && mImportedModules.find(module) == mImportedModules.end())
            {
                std::string realName;
                for (auto& name : import->getModule())
                {
                    realName += name + "::";
                }
                realName.pop_back();
                realName.pop_back();
                mDiag.reportCompilerError(
                    import->getSourcePair().start,
                    import->getSourcePair().end,
                    std::format("could not find module '{}{}{}'",
                        fmt::bold, realName, fmt::defaults)
                );
                std::exit(1);
            }
        }
    }

    for (auto& module : modules)
    {
        for (auto& file : mModules[module])
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
        if (mImportedModules.find(module) != mImportedModules.end())
        {
            for (auto& func : mImportedModules[module])
            {
                if (auto cloned = func->cloneExternal(mCUs[inputFilePath].globalScope.get()))
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

    mCUs[inputFilePath].module = vipir::Module(mCUs[inputFilePath].pathString);

    vipir::DIBuilder diBuilder;
    diBuilder.setProducer("basilisk");
    diBuilder.setDirectory(inputFilePath.parent_path().string());
    diBuilder.setFilename(inputFilePath.filename().string());
    diBuilder.borrowTypes(&mDiBuilder);

    module.setABI<vipir::abi::SysV>();
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

void Builder::assembleOne(std::filesystem::path inputFilePath, std::filesystem::path outputFilePath)
{
    std::ifstream file(inputFilePath);
    std::stringstream buffer;
    buffer << file.rdbuf();

    std::string text = std::move(buffer).str();

    lexing::Lexer lexer(text); // vasm lexer
    auto tokens = lexer.lex();
    error::ErrorReporter errorReporter;
    parsing::Parser parser(inputFilePath.string(), tokens, errorReporter);
    auto instructions = parser.parse();
    
    codegen::ELFFormat output(inputFilePath.string());
    codegen::OpcodeBuilder builder(&output, inputFilePath.string());
    for (auto& inst : instructions)
    {
        inst->emit(builder, builder.getSection());
    }
    builder.patchForwardLabels();

    std::ofstream outFile(outputFilePath, std::ios::binary);
    output.print(outFile);
}