// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_DIAGNOSTIC_DIAGNOSTIC_H
#define BASILISK_FRAMEWORK_DIAGNOSTIC_DIAGNOSTIC_H 1

#include <string>
#include <unordered_map>
#include <vector>

namespace basilisk::lexer
{
    class SourceLocation;
}
using namespace basilisk;

namespace fmt
{
    constexpr std::string_view bold     = "\x1b[1m";
    constexpr std::string_view red      = "\x1b[31m";
    constexpr std::string_view yellow   = "\x1b[93m";
    constexpr std::string_view defaults = "\x1b[0m";
}

namespace diagnostic
{
    class Diagnostics
    {
    public:
        Diagnostics();

        void addText(std::string path, std::string_view text);
        void setWarning(bool enable, std::string_view warning);
        void disableAllWarnings();
        void setImported(bool imported);

        [[noreturn]] void fatalError(std::string_view message);

        void reportCompilerError(lexer::SourceLocation start, lexer::SourceLocation end, std::string_view message);
        void compilerWarning(std::string_view type, lexer::SourceLocation start, lexer::SourceLocation end, std::string_view message);

    private:
        std::unordered_map<std::string, std::string_view> mTexts;
        std::vector<std::string_view> mWarnings;
        bool mImported{ false };

        int getLinePosition(std::string_view text, int lineNumber);
    };
}

#endif // BASILISK_FRAMEWORK_DIAGNOSTIC_DIAGNOSTIC_H