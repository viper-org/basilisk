// Copyright 2025 solar-mist

#include "Compiler.h"

#include "diagnostic/Diagnostic.h"

#include "lexer/Lexer.h"
#include "lexer/Token.h"

#include "import/ImportManager.h"

#include "parser/Parser.h"

#include "type/Type.h"

#include <vipir/ABI/SysV.h>
#include <vipir/Pass/DefaultPass.h>

#include <filesystem>
#include <fstream>

Compiler::Compiler(std::vector<Option> options, diagnostic::Diagnostics& diag)
    : mOptions(std::move(options))
    , mDiag(diag)
{
}

void Compiler::compile()
{
    auto inputFiles = Option::GetInputFiles(mOptions);
    if (inputFiles.empty())
    {
        mDiag.fatalError("no input files");
        std::exit(1);
    }
    if (inputFiles.size() > 1 && !Option::GetOutputFile(mOptions).empty())
    {
        mDiag.fatalError(std::format("cannot specify '{}-o{}' with '{}--cobject{}' with multiple input files", fmt::bold, fmt::defaults, fmt::bold, fmt::defaults));
        std::exit(1);
    }
    for (auto inputFilePath : inputFiles)
    {
        std::filesystem::path fullInputFilePath = std::filesystem::current_path() / inputFilePath;
        std::string fullInputPathName = fullInputFilePath.string();
        std::string outputFilePath = inputFilePath + ".o";
        if (!Option::GetOutputFile(mOptions).empty())
        {
            outputFilePath = Option::GetOutputFile(mOptions);
        }

        std::ifstream inputFile(fullInputPathName);
        std::stringstream buffer;
        buffer << inputFile.rdbuf();
        std::string text = std::move(buffer).str();
        inputFile.close();

        vipir::Module module(inputFilePath);

        vipir::DIBuilder diBuilder;
        diBuilder.setProducer("basilisk");
        diBuilder.setDirectory(fullInputFilePath.parent_path().string());
        diBuilder.setFilename(fullInputFilePath.filename().string());

        mDiag.setText(text);


        lexer::Lexer lexer(text, fullInputPathName);
        auto tokens = lexer.lex();

        Type::Init(&diBuilder);

        ImportManager importManager;
        auto scope = std::make_unique<Scope>(nullptr);
        parser::Parser parser(tokens, mDiag, importManager, scope.get());
        auto ast = parser.parse();

        importManager.reportUnknownTypeErrors();
        Type::FinalizeDITypes();

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
        checkOne(scope.get());

        std::ofstream outputFile(outputFilePath);
        module.setOutputFormat(vipir::OutputFormat::ELF);
        module.emit(outputFile);

        Type::Reset();
    }
}