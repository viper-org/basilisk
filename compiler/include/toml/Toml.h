// Copyright 2025 solar-mist

#ifndef BASILISK_COMPILER_TOML_TOML_H
#define BASILISK_COMPILER_TOML_TOML_H 1

#include <string>
#include <unordered_map>


namespace toml
{
    struct TomlValue
    {
        std::string value;
    };

    class Parser
    {
    public:
        Parser(const std::string& text);

        std::unordered_map<std::string, TomlValue> parse();

    private:
        const std::string& mText;
    };
}

#endif // BASILISK_COMPILER_TOML_TOML_H