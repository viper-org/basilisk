// Copyright 2025 solar-mist

#ifndef BASILISK_COMPILER_LINKER_H
#define BASILISK_COMPILER_LINKER_H 1

#include "Options.h"

#include "diagnostic/Diagnostic.h"

#include <vector>

class Linker
{
public:
    Linker(std::vector<Option> options, diagnostic::Diagnostics& diag);

    void linkLibrary();

    void linkExecutable();

private:
    std::vector<Option> mOptions;

    diagnostic::Diagnostics& mDiag;
};

#endif // BASILISK_COMPILER_LINKER_H