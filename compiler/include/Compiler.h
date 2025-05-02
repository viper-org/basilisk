// Copyright 2025 solar-mist

#ifndef BASILISK_COMPILER_COMPILER_H
#define BASILISK_COMPILER_COMPILER_H 1

#include "Options.h"

#include "diagnostic/Diagnostic.h"

#include <vector>

class Compiler
{
public:
    Compiler(std::vector<Option> options, diagnostic::Diagnostics& diag);

    void compile();

private:
    std::vector<Option> mOptions;

    diagnostic::Diagnostics& mDiag;
};

#endif // BASILISK_COMPILER_COMPILER_H