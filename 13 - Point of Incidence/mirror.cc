#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

#include "simpleparser.hpp"

using std::views::iota;

struct Valley /* of mirrors */ {
    std::vector<std::string> mirrors;
    std::vector<std::string> flipped;

    Valley(std::ifstream &scan) {
        while (!scan.eof()) {
            std::string line;
            std::getline(scan, line);
            if (line.size() == 0) {
                break;
            }
            mirrors.push_back(line);
        }

        for (const auto x : iota(0u, mirrors[0].size())) {
            flipped.emplace_back("");
            for (const auto y : iota(0u, mirrors.size())) {
                flipped.at(x) += mirrors.at(y)[x];
            }
        }
    }

    static int64_t diffCount(const std::string &lhs, const std::string &rhs) {
        int64_t diff = 0;
        for (const auto pos : iota(0u, lhs.size())) {
            if (lhs[pos] != rhs[pos]) {
                ++diff;
            }
        }
        return diff;
    }

    static int64_t findSmudgedReflection(const int64_t smudgesWanted, const auto &mirrors) {
        for (const auto reflect : iota(0u, mirrors.size() - 1)) {
            int64_t smudges = 0;
            for (const auto offset : iota(0u, mirrors.size() / 2)) {
                if (reflect - offset < mirrors.size() and reflect + 1 + offset < mirrors.size()) {
                    smudges += diffCount(mirrors[reflect - offset], mirrors[reflect + 1 + offset]);
                    if (smudges > smudgesWanted) {
                        break;
                    }
                }
            }
            if (smudges == smudgesWanted) {
                return reflect + 1;
            }
        }
        return 0;
    }

    int64_t findSmudgedReflection(const int64_t smudgesWanted = 1) const {
        const int64_t vertical = findSmudgedReflection(smudgesWanted, flipped);
        const int64_t horizontal = findSmudgedReflection(smudgesWanted, mirrors);
        return vertical + horizontal * 100;
    }
};

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    std::vector<Valley> island{};
    std::ifstream scan{argv[1]};
    while (!scan.eof()) {
        island.emplace_back(scan);
    }

    int64_t sum = 0;
    int64_t realSum = 0;

    for (const auto &gorge : island) {
        const auto reflect = gorge.findSmudgedReflection(0);
        const auto realflect = gorge.findSmudgedReflection(1);
        fmt::print("Valley values {}, {}\n", reflect, realflect);
        sum += reflect;
        realSum += realflect;
    }
    fmt::print("Total sum: {}\n", sum);
    fmt::print("Sum through the looking glass: {}\n", realSum);
}
