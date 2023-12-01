#include <algorithm>
#include <cctype>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include "simpleparser.hpp"

using std::views::iota;

const std::array<std::pair<const std::string, const int>, 9> digits = {
    {{"one", 1},
     {"two", 2},
     {"three", 3},
     {"four", 4},
     {"five", 5},
     {"six", 6},
     {"seven", 7},
     {"eight", 8},
     {"nine", 9}}};

void part1(const char *name) {
    std::ifstream infile{name};
    std::string line;
    int sum = 0;
    while (std::getline(infile, line)) {
        std::optional<int> first{};
        int last{};
        for (const auto digit : line) {
            if (std::isdigit(digit)) {
                last = digit - '0';
                if (!first) {
                    first = last;
                }
            }
        }
        sum += *first * 10 + last;
    }
    fmt::print("The calibration summs to {}.\n", sum);
}

void part2(const char *name) {
    std::ifstream infile{name};
    std::string line_in;
    int sum = 0;
    while (std::getline(infile, line_in)) {
        const std::string_view line{line_in};
        std::optional<int> first{};
        int last{};
        for (const auto pos : iota(0u, line.size())) {
            if (std::isdigit(line[pos])) {
                last = line[pos] - '0';
                if (!first) {
                    first = last;
                }
            } else {
                for (const auto &[digstr, digval] : digits) {
                    if (line.substr(pos).starts_with(digstr)) {
                        last = digval;
                        if (!first) {
                            first = last;
                        }
                        break;
                    }
                }
            }
        }
        sum += *first * 10 + last;
    }
    fmt::print("The corrected calibration summs to {}.\n", sum);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }
    part1(argv[1]);
    part2(argv[1]);
}
