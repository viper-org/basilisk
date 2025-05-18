// Copyright 2025 solar-mist

#include "Builder.h"

using namespace std::literals;

int main(int argc, char** argv)
{
    diagnostic::Diagnostics diag;

    if (argv[1] == "build"s)
    {
        Builder builder = Builder(diag);
        builder.build();
    }
    else if (argv[1] == "run"s)
    {
        diag.disableAllWarnings();
        Builder builder = Builder(diag);
        builder.build();
        execl(builder.getOutputFile().c_str(), builder.getOutputFile().c_str(), nullptr);
    }
    else
    {
        diag.fatalError("unknown command");
        std::exit(1);
    }

    return 0;
}