#include <algorithm>
#include <cctype>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

#include "simpleparser.hpp"

using std::views::iota;

void part1(const char *name) {
    std::ifstream infile{name};
    std::string line;
    int sum = 0;
    while (std::getline(infile, line)) {
        int first = -1;
        int last = -1;
        for (const auto digit : line) {
            if (std::isdigit(digit)) {
                last = digit - '0';
                if (first == -1) {
                    first = last;
                }
            }
        }
        sum += first * 10 + last;
    }
    fmt::print("The calibration summs to {}.\n", sum);
}

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

void part2(const char *name) {
    std::ifstream infile{name};
    std::string line;
    int sum = 0;
    while (std::getline(infile, line)) {
        int first = -1;
        int last = -1;
        for (const auto pos : iota(0u, line.size())) {
            if (std::isdigit(line[pos])) {
                last = line[pos] - '0';
                if (first == -1) {
                    first = last;
                }
            } else {
                for (const auto &[digstr, digval] : digits) {
                    if (line.substr(pos).starts_with(digstr)) {
                        last = digval;
                        if (first == -1) {
                            first = last;
                        }
                    }
                }
            }
        }
        sum += first * 10 + last;
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
