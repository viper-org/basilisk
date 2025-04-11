// Copyright 2025 solar-mist

#include <lexer/Lexer.h>
#include <lexer/Token.h>

#include <parser/Parser.h>

#include <vipir/Module.h>
#include <vipir/ABI/SysV.h>

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

    std::string inputFilePath = argv[1]; // TODO: Command-line option parsing
    std::ifstream inputFile(inputFilePath);
    std::stringstream buffer;
    buffer << inputFile.rdbuf();
    std::string text = std::move(buffer).str();
    inputFile.close();

    lexer::Lexer lexer(text, inputFilePath);
    auto tokens = lexer.lex();

    diagnostic::Diagnostics diag;
    diag.setText(text);
    
    parser::Parser parser(tokens, diag);
    auto ast = parser.parse();

    vipir::Module module(inputFilePath);
    module.setABI<vipir::abi::SysV>();
    vipir::IRBuilder builder;

    for (auto& node : ast)
    {
        node->codegen(builder, module);
    }

    std::ofstream outputFile(inputFilePath + ".o"s);
    module.setOutputFormat(vipir::OutputFormat::ELF);
    module.emit(outputFile);

    return 0;
}