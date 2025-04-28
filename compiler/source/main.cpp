// Copyright 2025 solar-mist

#include "Options.h"

#include <lexer/Lexer.h>
#include <lexer/Token.h>

#include <parser/Parser.h>

#include <vipir/Module.h>
#include <vipir/ABI/SysV.h>
#include <vipir/Pass/DefaultPass.h>
#include <vipir/Pass/IRInfoPass.h>

#include <fstream>
#include <iostream>
#include <sstream>

using namespace std::literals;

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "basilisk: no input files\n";
        std::exit(1);
    }

    auto options = Option::ParseOptions(argc, argv);
    auto inputFilePath = Option::GetInputFile(options);
    std::filesystem::path fullInputFilePath = std::filesystem::current_path() / inputFilePath;
    std::string fullInputPathName = fullInputFilePath.string();

    std::ifstream inputFile(fullInputPathName);
    std::stringstream buffer;
    buffer << inputFile.rdbuf();
    std::string text = std::move(buffer).str();
    inputFile.close();

    std::string outputFilePath = inputFilePath + ".o"s;

    vipir::Module module(inputFilePath);
    bool generateDebugInfo = false;
    vipir::DIBuilder diBuilder(module);
    diBuilder.setProducer("basilisk compiler");
    diBuilder.setDirectory(fullInputFilePath.parent_path().string());
    diBuilder.setFilename(fullInputFilePath.filename().string());

    diagnostic::Diagnostics diag;
    diag.setText(text);
    for (const auto& option : options)
    {
        if (option.type == OptionType::WarningSpec)
        {
            if (option.value.starts_with("no-"))
                diag.setWarning(false, option.value.substr(3));
            else
                diag.setWarning(true, option.value);
        }
        else if (option.type == OptionType::DebugInfoEmission)
        {
            module.getPassManager().addPass(std::make_unique<vipir::DebugInfoEmissionPass>(&diBuilder));
            generateDebugInfo = true;
        }
        else if (option.type == OptionType::OutputFile)
        {
            outputFilePath = option.value;
        }
    }


    lexer::Lexer lexer(text, fullInputPathName);
    auto tokens = lexer.lex();

    Type::Init(&diBuilder);

    ImportManager importManager;
    parser::Parser parser(tokens, diag, importManager);
    auto ast = parser.parse();

    importManager.reportUnknownTypeErrors();
    
    Type::FinalizeDITypes();

    module.setABI<vipir::abi::SysV>();
    Option::ParseOptimizingFlags(options, module, diag);
    // Always do constant folding
    if (module.getPassManager().findPass(vipir::PassType::ConstantFolding) == -1)
        module.getPassManager().insertBefore(vipir::PassType::LIREmission, std::make_unique<vipir::ConstantFoldingPass>());

    vipir::IRBuilder builder;
    
    bool exit = false;
    for (auto& node : ast)
    {
        node->typeCheck(diag, exit);
    }
    if (exit) return 1;

    for (auto& node : ast)
    {
        node->codegen(builder, diBuilder, module, diag);
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
    checkOne(Scope::GetGlobalScope());

    std::ofstream outputFile(outputFilePath);
    std::ofstream IROutputFile(inputFilePath + ".vipir"s);
    module.print(IROutputFile);
    module.setOutputFormat(vipir::OutputFormat::ELF);
    module.emit(outputFile);

    return 0;
}