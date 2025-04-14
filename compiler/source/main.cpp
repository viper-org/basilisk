// Copyright 2025 solar-mist

#include "Options.h"

#include <lexer/Lexer.h>
#include <lexer/Token.h>

#include <parser/Parser.h>

#include <vipir/Module.h>
#include <vipir/ABI/SysV.h>
#include <vipir/Pass/DefaultPass.h>

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

    std::ifstream inputFile(inputFilePath);
    std::stringstream buffer;
    buffer << inputFile.rdbuf();
    std::string text = std::move(buffer).str();
    inputFile.close();

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
    }


    lexer::Lexer lexer(text, inputFilePath);
    auto tokens = lexer.lex();

    Type::Init();
    
    parser::Parser parser(tokens, diag);
    auto ast = parser.parse();

    vipir::Module module(inputFilePath);
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
        node->codegen(builder, module, diag);
    }

    std::ofstream outputFile(inputFilePath + ".o"s);
    std::ofstream IROutputFile(inputFilePath + ".vipir"s);
    module.print(IROutputFile);
    module.setOutputFormat(vipir::OutputFormat::ELF);
    module.emit(outputFile);

    return 0;
}