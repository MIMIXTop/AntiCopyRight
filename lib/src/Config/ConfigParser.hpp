#pragma once
#include <unordered_map>
#include <string>

namespace Util {
    class ConfigParser {
    public:
        ConfigParser();

        ~ConfigParser() = default;

        std::string_view operator[](const std::string &key) const;

    private:
        std::unordered_map<std::string, std::string> variables;
    };
} // Util
