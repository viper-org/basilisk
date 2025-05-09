// Copyright 2025 solar-mist

#include "Linker.h"

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
        if (!inputFile.ends_with(".o"))
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

    // TODO: Use ld and link with standard library
    std::string command = "gcc -o " + outputFile + " " + inputFileConcat;
    int ret = std::system(command.c_str());
    if (ret != 0)
    {
        mDiag.fatalError(std::format("linker command failed with error code {}", ret));
        std::exit(ret);
    }
}