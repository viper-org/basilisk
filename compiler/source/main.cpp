// Copyright 2025 solar-mist

#include <lexer/Lexer.h>
#include <lexer/Token.h>

#include <fstream>
#include <iostream>
#include <sstream>

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
    for (auto& token : tokens)
    {
        std::cout << token.getName() << "\n";
    }

    return 0;
}