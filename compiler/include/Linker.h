// Copyright 2025 solar-mist

#ifndef BASILISK_COMPILER_LINKER_H
#define BASILISK_COMPILER_LINKER_H 1

#include "diagnostic/Diagnostic.h"

#include <vector>

class Linker
{
public:
    Linker(std::vector<std::string> inputFiles, std::vector<std::string> libraries, std::string outputFile, diagnostic::Diagnostics& diag);

    void linkLibrary();

    void linkExecutable();

private:
    std::vector<std::string> mInputFiles;
    std::vector<std::string> mLibraries;
    std::string mOutputFile;

    diagnostic::Diagnostics& mDiag;
};

#endif // BASILISK_COMPILER_LINKER_H