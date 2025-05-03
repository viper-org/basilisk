// Copyright 2025 solar-mist

#ifndef BASILISK_COMPILER_BUILDER_H
#define BASILISK_COMPILER_BUILDER_H 1

#include "Options.h"

#include "diagnostic/Diagnostic.h"

#include <filesystem>

class Builder
{
public:
    Builder(std::vector<Option> options, diagnostic::Diagnostics& diag);

    void build();

private:
    std::vector<Option> mOptions;

    diagnostic::Diagnostics& mDiag;

    std::vector<std::filesystem::path> mObjects;

    void compileObjects(std::filesystem::path projectDir);
    void linkExecutable(std::filesystem::path projectDir);
};

#endif // BASILISK_COMPILER_BUILDER_H