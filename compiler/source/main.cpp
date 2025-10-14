// Copyright 2025 solar-mist

#include "Builder.h"

#include "util/Process.h"

using namespace std::literals;

int main(int argc, char** argv)
{
    diagnostic::Diagnostics diag;

    if (argc == 1)
    {
        // Assume builder for now
        Builder builder = Builder(diag);
        builder.build();
        return 0;
        //diag.fatalError("no command provided");
    }

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
        util::ExecuteProcess(builder.getOutputFile());
    }
    else
    {
        diag.fatalError("unknown command");
        std::exit(1);
    }

    return 0;
}