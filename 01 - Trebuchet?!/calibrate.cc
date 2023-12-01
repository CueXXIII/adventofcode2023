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

std::pair<int, int> scan_left(const std::string_view &line) {
    std::optional<int> any_digit{};
    for (const auto pos : iota(0u, line.size())) {
        if (std::isdigit(line[pos])) {
            const int numeric_digit = line[pos] - '0';
            if (!any_digit) {
                any_digit = numeric_digit;
            }
            return {numeric_digit, *any_digit};
        } else if (!any_digit) {
            for (const auto &[digstr, digval] : digits) {
                if (line.substr(pos).starts_with(digstr)) {
                    any_digit = digval;
                    break;
                }
            }
        }
    }
    return {0, any_digit.value_or(0)};
}

std::pair<int, int> scan_right(const std::string_view &line) {
    std::optional<int> any_digit{};
    for (auto pos = line.size() - 1; pos + 1 != 0; --pos) {
        if (std::isdigit(line[pos])) {
            const int numeric_digit = line[pos] - '0';
            if (!any_digit) {
                any_digit = numeric_digit;
            }
            return {numeric_digit, *any_digit};
        } else if (!any_digit) {
            for (const auto &[digstr, digval] : digits) {
                if (line.substr(pos).starts_with(digstr)) {
                    any_digit = digval;
                    break;
                }
            }
        }
    }
    return {0, any_digit.value_or(0)};
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }
    std::ifstream infile{argv[1]};
    std::string line;
    int sum1 = 0;
    int sum2 = 0;
    while (std::getline(infile, line)) {
        const std::string_view line_v{line};
        const auto [part1left, part2left] = scan_left(line_v);
        const auto [part1right, part2right] = scan_right(line_v);

        sum1 += part1left * 10 + part1right;
        sum2 += part2left * 10 + part2right;
    }
    fmt::print("The calibration summs to {}.\n", sum1);
    fmt::print("The corrected calibration summs to {}.\n", sum2);
}
