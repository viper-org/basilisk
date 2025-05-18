// Copyright 2025 solar-mist

#include "diagnostic/Diagnostic.h"

#include "lexer/SourceLocation.h"

#include <algorithm>
#include <format>
#include <iostream>

namespace diagnostic
{
    constexpr std::array knownWarnings = {
        "implicit",
        "unused"
    };

    Diagnostics::Diagnostics()
    {
        mWarnings = {
            "implicit",
            "unused"
        };
    }

    void Diagnostics::addText(std::string path, std::string_view text)
    {
        mTexts[path] = text;
    }

    void Diagnostics::setWarning(bool enable, std::string_view warning)
    {
        auto warningIt = std::find(knownWarnings.begin(), knownWarnings.end(), warning);
        if (warningIt == knownWarnings.end())
        {
            fatalError(std::format("unknown warning: -W{}{}", enable?"":"no-", warning));
        }

        auto it = std::find(mWarnings.begin(), mWarnings.end(), warning);
        if (enable && it == mWarnings.end()) mWarnings.push_back(warning);
        if (!enable && it != mWarnings.end()) mWarnings.erase(it);
    }

    void Diagnostics::disableAllWarnings()
    {
        mWarnings.clear();
    }

    void Diagnostics::setImported(bool imported)
    {
        mImported = imported;
    }


    void Diagnostics::fatalError(std::string_view message)
    {
        std::cerr << std::format("{}basilisk: {}fatal error: {}{}\n", fmt::bold, fmt::red, fmt::defaults, message);

        std::exit(EXIT_FAILURE);
    }

    void Diagnostics::reportCompilerError(lexer::SourceLocation start, lexer::SourceLocation end, std::string_view message)
    {
        auto text = mTexts[std::string(start.file)];

        int lineStart = getLinePosition(text, start.line-1);
        int lineEnd = getLinePosition(text, end.line)-1;

        end.position += 1;
        std::string before = std::string(text.substr(lineStart, start.position - lineStart));
        std::string_view error = text.substr(start.position, end.position - start.position);
        std::string_view after = text.substr(end.position, lineEnd - end.position);
        std::string spacesBefore = std::string(std::to_string(start.line).length(), ' ');
        std::string spacesAfter = std::string(before.length(), ' ');

        std::for_each(before.begin(), before.end(), [](char& c){if(c == '\t')c=' ';});

        std::cerr << std::format("{}{}:{}:{} {}error: {}{}\n", fmt::bold, start.file, start.line, start.col, fmt::red, fmt::defaults, message);
        std::cerr << std::format("    {} | {}{}{}{}{}{}\n", start.line, before, fmt::bold, fmt::red, error, fmt::defaults, after);
        std::cerr << std::format("    {} | {}{}{}^{}{}\n", spacesBefore, spacesAfter, fmt::bold, fmt::red, std::string(error.length()-1, '~'), fmt::defaults);
    }

    void Diagnostics::compilerWarning(std::string_view type, lexer::SourceLocation start, lexer::SourceLocation end, std::string_view message)
    {
        auto it = std::find(mWarnings.begin(), mWarnings.end(), type);
        if (it == mWarnings.end()) return;

        auto text = mTexts[std::string(start.file)];

        int lineStart = getLinePosition(text, start.line-1);
        int lineEnd = getLinePosition(text, end.line)-1;

        end.position += 1;
        std::string_view before = text.substr(lineStart, start.position - lineStart);
        std::string_view error = text.substr(start.position, end.position - start.position);
        std::string_view after = text.substr(end.position, lineEnd - end.position);
        std::string spacesBefore = std::string(std::to_string(start.line).length(), ' ');
        std::string spacesAfter = std::string(before.length(), ' ');

        std::cerr << std::format("{}{}:{}:{} {}warning: {}{}", fmt::bold, start.file, start.line, start.col, fmt::yellow, fmt::defaults, message);
        std::cerr << std::format(" [{}{}-W{}{}]\n", fmt::bold, fmt::yellow, type, fmt::defaults);
        std::cerr << std::format("    {} | {}{}{}{}{}{}\n", start.line, before, fmt::bold, fmt::yellow, error, fmt::defaults, after);
        std::cerr << std::format("    {} | {}{}{}^{}{}\n", spacesBefore, spacesAfter, fmt::bold, fmt::yellow, std::string(error.length()-1, '~'), fmt::defaults);
    }


    int Diagnostics::getLinePosition(std::string_view text, int lineNumber)
    {
        int line = 0;
        for (int i = 0; i < lineNumber; ++i)
        {
            while(text[line] != '\n')
            {
                ++line;
            }
            ++line;
        }
        return line;
    }
}