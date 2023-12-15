#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

#include "simpleparser.hpp"

using std::views::iota;

int64_t hash(const std::string msg) {
    int64_t current = 0;
    for (const auto c : msg) {
        current = ((current + c) * 17) % 256;
    }
    return current;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    int64_t sum = 0;
    SimpleParser scan{argv[1]};
    while (!scan.isEof()) {
        const std::string step = scan.getToken(',');
        scan.skipChar(',');
        const auto result = hash(step);
        fmt::print("'{}' = {}\n", step, result);
        sum += result;
    }
    fmt::print("The initialization HASH is {}\n", sum);
}
