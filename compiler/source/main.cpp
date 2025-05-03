// Copyright 2025 solar-mist

#include "Options.h"

#include "Linker.h"
#include "Compiler.h"
#include "Builder.h"

#include <iostream>

using namespace std::literals;

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "basilisk: no input files\n";
        std::exit(1);
    }

    auto options = Option::ParseOptions(argc, argv);

    diagnostic::Diagnostics diag;
    std::string driverMode = "object";
    for (const auto& option : options)
    {
        if (option.type == OptionType::WarningSpec)
        {
            if (option.value.starts_with("no-"))
                diag.setWarning(false, option.value.substr(3));
            else
                diag.setWarning(true, option.value);
        }
        else if (option.type == OptionType::DriverMode)
        {
            driverMode = option.value;
        }
    }

    if (driverMode == "object")
    {
        Compiler compiler(options, diag);
        compiler.compile();
    }
    else if (driverMode == "exec")
    {
        Linker linker(options, diag);
        linker.linkExecutable();
    }
    else if (driverMode == "build")
    {
        Builder builder(options, diag);
        builder.build();
    }
    else
    {
        diag.fatalError(std::format("unknown driver mode '{}'", driverMode));
        std::exit(1);
    }

    return 0;
}