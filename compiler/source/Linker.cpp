// Copyright 2025 solar-mist

#include "Linker.h"

#include "util/Linker.h"

#include <filesystem>
#include <format>

Linker::Linker(std::vector<std::string> inputFiles, std::vector<std::string> libraries, std::string outputFile, diagnostic::Diagnostics& diag)
    : mInputFiles(std::move(inputFiles))
    , mLibraries(std::move(libraries))
    , mOutputFile(std::move(outputFile))
    , mDiag(diag)
{
}

void Linker::linkLibrary()
{
    if (mOutputFile.empty()) mOutputFile = "a.out";

    if (mInputFiles.empty())
    {
        mDiag.fatalError("no input files");
        std::exit(1);
    }

    std::string inputFileConcat;
    for (auto inputFile : mInputFiles)
    {
        if (!inputFile.ends_with(".o") && !inputFile.ends_with(".a"))
        {
            mDiag.fatalError(std::format("file '{}{}{}' has unrecognized file format", fmt::bold, inputFile, fmt::defaults));
            std::exit(1);
        }
        inputFileConcat += inputFile + " ";
    }

    std::string command = "ar -rcs " + mOutputFile + " " + inputFileConcat;
    int ret = std::system(command.c_str());
    if (ret != 0)
    {
        mDiag.fatalError(std::format("linker command failed with error code {}", ret));
        std::exit(ret);
    }
}

void Linker::linkExecutable()
{
    if (mOutputFile.empty()) mOutputFile = "a.out";

    if (mInputFiles.empty())
    {
        mDiag.fatalError("no input files");
        std::exit(1);
    }

    std::string inputFileConcat;
    for (auto inputFile : mInputFiles)
    {
        if (!inputFile.ends_with(".o"))
        {
            mDiag.fatalError(std::format("file '{}{}{}' has unrecognized file format", fmt::bold, inputFile, fmt::defaults));
            std::exit(1);
        }
        inputFileConcat += inputFile + " ";
    }
    for (auto library : mLibraries)
    {
        std::filesystem::path libPath = library;
        inputFileConcat += "-L " + libPath.parent_path().string();

        inputFileConcat += " -l:" + libPath.filename().string() + " ";
    }

    // TODO: Use ld and link with standard library
    auto linker = util::FindLinker();
	std::string command = util::EncodeCommand(linker.string(), mOutputFile, inputFileConcat);
    int ret = std::system(command.c_str());
    if (ret != 0)
    {
        mDiag.fatalError(std::format("linker command failed with error code {}", ret));
        std::exit(ret);
    }
}