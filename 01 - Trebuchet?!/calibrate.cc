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

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    std::ifstream infile{argv[1]};
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
