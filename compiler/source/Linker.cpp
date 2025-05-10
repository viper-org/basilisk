// Copyright 2025 solar-mist

#include "Linker.h"
#include <iostream>

#include <filesystem>

Linker::Linker(std::vector<Option> options, diagnostic::Diagnostics& diag)
    : mOptions(std::move(options))
    , mDiag(diag)
{
}

void Linker::linkLibrary()
{
    // TODO: Symbols as well as object files
    auto inputFiles = Option::GetInputFiles(mOptions);
    auto outputFile = Option::GetOutputFile(mOptions);
    if (outputFile.empty()) outputFile = "a.out";

    if (inputFiles.empty())
    {
        mDiag.fatalError("no input files");
        std::exit(1);
    }

    std::string inputFileConcat;
    for (auto inputFile : inputFiles)
    {
        if (!inputFile.ends_with(".o") && !inputFile.ends_with(".a"))
        {
            mDiag.fatalError(std::format("file '{}{}{}' has unrecognized file format", fmt::bold, inputFile, fmt::defaults));
            std::exit(1);
        }
        inputFileConcat += inputFile + " ";
    }

    std::string command = "ar -rcs " + outputFile + " " + inputFileConcat;
    int ret = std::system(command.c_str());
    if (ret != 0)
    {
        mDiag.fatalError(std::format("linker command failed with error code {}", ret));
        std::exit(ret);
    }
}

void Linker::linkExecutable()
{
    auto inputFiles = Option::GetInputFiles(mOptions);
    auto outputFile = Option::GetOutputFile(mOptions);
    if (outputFile.empty()) outputFile = "a.out";

    if (inputFiles.empty())
    {
        mDiag.fatalError("no input files");
        std::exit(1);
    }

    std::string inputFileConcat;
    for (auto inputFile : inputFiles)
    {
        if (!inputFile.ends_with(".o"))
        {
            mDiag.fatalError(std::format("file '{}{}{}' has unrecognized file format", fmt::bold, inputFile, fmt::defaults));
            std::exit(1);
        }
        inputFileConcat += inputFile + " ";
    }
    for (auto option : mOptions)
    {
        if (option.type == OptionType::InputLibrary)
        {
            std::filesystem::path libPath = option.value;
            inputFileConcat += "-L " + libPath.parent_path().string();

            inputFileConcat += " -l:" + libPath.filename().string() + " ";
        }
    }

    // TODO: Use ld and link with standard library
    std::string command = "gcc -o " + outputFile + " " + inputFileConcat;
    std::cout << command << std::endl;
    int ret = std::system(command.c_str());
    if (ret != 0)
    {
        mDiag.fatalError(std::format("linker command failed with error code {}", ret));
        std::exit(ret);
    }
}