// Copyright 2025 solar-mist

#include "Builder.h"
#include "Compiler.h"
#include "Linker.h"

#include <filesystem>

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

    // TODO: Add module scanning

    compileObjects(projectDir);
    // TODO: Read bsproj file to determine build type
    linkExecutable(projectDir);
}


void Builder::compileObjects(std::filesystem::path projectDir)
{
    std::filesystem::path sourceDir = projectDir / "src";
    std::filesystem::path buildDir = projectDir / "build";
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

            std::vector<Option> compileOptions;
            compileOptions.push_back({OptionType::InputFile, inputFile.string()});
            compileOptions.push_back({OptionType::OutputFile, outputFile.string()});
            Compiler compiler(compileOptions, mDiag);
            compiler.compile();
        }
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