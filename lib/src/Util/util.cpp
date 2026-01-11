#include "util.hpp"

#include <algorithm>
#include <ranges>

namespace util {

std::string StringWorker::getFirstLemma(std::string& line) {
    auto it = std::ranges::find(line, '|');
    if (it == line.end()) {
        line.erase(std::ranges::remove(line, '\n').begin(), line.end());
        return line;
    }
    return std::string(line.begin(), it);
}
}   // namespace util
