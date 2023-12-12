// Forward Spring Correction
#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include "simpleparser.hpp"

using std::views::iota;

struct Area {
    int64_t len = 0;
    std::string layout{};
    bool hasSpring = false;
};

struct SpringRow {
    std::vector<Area> areas{};
    std::vector<int64_t> broken{};
    std::string springs{};

    SpringRow(SimpleParser &scan) {
        springs = scan.getToken();
        springs = springs + '?' + springs + '?' + springs + '?' + springs + '?' + springs;

        size_t pos = 0;
        while (pos < springs.size()) {
            Area current;
            while (pos < springs.size() and springs[pos] != '.') {
                current.layout += springs[pos];
                current.hasSpring |= springs[pos] == '#';
                ++pos;
                ++current.len;
            }
            if (current.len > 0) {
                areas.push_back(current);
            }
            ++pos;
        }

        std::vector<int64_t> bpart{};
        while (!scan.isEol()) {
            const auto n = scan.getInt64();
            bpart.push_back(n);
            if (!scan.skipChar(',')) {
                break;
            }
        }
        broken.insert(broken.end(), bpart.begin(), bpart.end());
        broken.insert(broken.end(), bpart.begin(), bpart.end());
        broken.insert(broken.end(), bpart.begin(), bpart.end());
        broken.insert(broken.end(), bpart.begin(), bpart.end());
        broken.insert(broken.end(), bpart.begin(), bpart.end());
    }

    static int64_t canSkip(const std::string_view &layout, const size_t size) {
        const auto result = !layout.substr(0u, size).contains('#');
        // fmt::print("canSkip('{}', {}) = {}\n", layout, size, result);
        return result;
    }

    static int64_t canBeBroken(const std::string_view &layout, const size_t size) {
        const auto result = layout.size() >= size;
        // fmt::print("canBeBroken('{}', {}) = {}\n", layout, size, result);
        return result;
    }

    int64_t countFits(const std::string_view layout, const size_t bStart, const size_t bEnd) const {
        // fmt::print("countFits broken[{}, {}) into '{}')...\n", bStart, bEnd, layout);
        int64_t sum = 0;
        // TODO memo

        if (bStart >= bEnd) {
            if (canSkip(layout, layout.size())) {
                sum = 1;
                goto log;
            } else {
                sum = 0;
                goto log;
            }
        }

        for (size_t aPos = 0; aPos < layout.size(); ++aPos) {
            if (canSkip(layout, aPos)) {
                if (canBeBroken(layout.substr(aPos), broken[bStart])) {
                    const auto layoutBehind = layout.substr(aPos + broken[bStart]);
                    if (layoutBehind.size() == 0) {
                        sum += countFits("", bStart + 1, bEnd);
                    } else if (canSkip(layoutBehind, 1)) {
                        sum += countFits(layoutBehind.substr(1), bStart + 1, bEnd);
                    }
                }
            }
        }

    goto log;

        fmt::print("countFits [");
        if (bStart < bEnd) {
            fmt::print("{}", broken[bStart]);
            for (const auto pos : iota(bStart + 1, bEnd)) {
                fmt::print(", {}", broken[pos]);
            }
        }
        fmt::print("] into '{}') = {}\n", layout, sum);

    log:
        return sum;
    }

    // aPos: position in areas that can contains springs
    // bPos: position in broken number of springs
    // arrange() is always called after a spring was complete
    int64_t arrange(const size_t aPos = 0, const size_t bPos = 0) const {
        // fmt::print("arrange({}, {})\n", aPos, bPos);
        if (aPos >= areas.size()) {
            if (bPos >= broken.size()) {
                return 1;
            } else {
                return 0;
            }
        }

        int64_t found = 0;
        size_t brokenSize = 0;
        for (const auto bEnd : iota(bPos, broken.size() + 1)) {
            // fmt::print("arrange ({}, {}) bEnd={}\n", aPos, bPos, bEnd);
            const auto fits = countFits(areas[aPos].layout, bPos, bEnd);
            if (fits > 0) {
                found += fits * arrange(aPos + 1, bEnd);
            }
            // brokenSize += (brokenSize == 0 ? 0 : 1) + broken[n];
            // if (brokenSize > areas[aPos].layout.size()) {
            //     break;
            // }
        }
        return found;
    }
};

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    std::vector<SpringRow> hotTub{};
    SimpleParser scan{argv[1]};
    while (!scan.isEof()) {
        hotTub.emplace_back(scan);
    }

    int n = 0;
    int64_t sum = 0;
    for (auto &row : hotTub) {
        const auto value = row.arrange();
        sum += value;
        fmt::print("{:4}: {} has {} arrangements\n", ++n, row.springs, value);
    }
    fmt::print("The arrangements sum is {}\n", sum);
}
