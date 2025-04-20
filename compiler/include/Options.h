// Copyright 2025 solar-mist

#ifndef BASILISK_COMPILER_OPTIONS_H
#define BASILISK_COMPILER_OPTIONS_H 1

#include "diagnostic/Diagnostic.h"

#include "vipir/Module.h"

#include <string>
#include <vector>

enum class OptionType
{
    WarningSpec,
    FlagSpec,
    OptimizationLevelSpec,
    InputFile,
    DebugInfoEmission,
    OutputFile
};

struct Option
{
    OptionType type;
    std::string value;

    static std::vector<Option> ParseOptions(int argc, char** argv);
    static std::string GetInputFile(const std::vector<Option>& options);

    static void ParseOptimizingFlags(const std::vector<Option>& options, vipir::Module& module, diagnostic::Diagnostics& diag);
};

#endif // BASILISK_COMPILER_OPTIONS_H