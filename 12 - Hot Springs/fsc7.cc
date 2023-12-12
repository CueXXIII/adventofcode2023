// Forward Spring Correction
#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "simpleparser.hpp"

using std::views::iota;

std::unordered_map<std::string, int64_t> MEMO{};

struct Area {
    int64_t len = 0;
    std::string layout{};
    bool hasSpring = false;
};

struct SpringRow {
    std::vector<Area> areas{};
    std::vector<int64_t> broken{};
    std::string springs{};
    int64_t id{};

    SpringRow(SimpleParser &scan, const int64_t id) : id(id) {
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

    std::string MEMOcountFits(const auto &layout, const auto bStart, const auto bEnd) const {
        std::string result{layout};
        for (const auto bPos : iota(bStart, bEnd)) {
            result += fmt::format(":{}", broken[bPos]);
        }
        return result;
    }

    std::string MEMOarrange(const auto id, const auto aPos, const auto bPos) const {
        std::string result{fmt::format("{};{};{}", id, aPos, bPos)};
        return result;
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

        const auto MEMOid = MEMOcountFits(layout, bStart, bEnd);
        if (MEMO.contains(MEMOid)) {
            return MEMO[MEMOid];
        }

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
        MEMO[MEMOid] = sum;
        return sum;
    }

    // aPos: position in areas that can contains springs
    // bPos: position in broken number of springs
    // arrange() is always called after a spring was complete
    int64_t arrange(const size_t aPos = 0, const size_t bPos = 0) const {
        // fmt::print("arrange({}, {})\n", aPos, bPos);

        const auto MEMOid = MEMOarrange(id, aPos, bPos);

        if (MEMO.contains(MEMOid)) {
            return MEMO[MEMOid];
        }

        if (aPos >= areas.size()) {
            if (bPos >= broken.size()) {
                MEMO[MEMOid] = 1;
                return 1;
            } else {
                MEMO[MEMOid] = 0;
                return 0;
            }
        }

        int64_t found = 0;
        for (const auto bEnd : iota(bPos, broken.size() + 1)) {
            // fmt::print("arrange ({}, {}) bEnd={}\n", aPos, bPos, bEnd);
            const auto fits = countFits(areas[aPos].layout, bPos, bEnd);
            if (fits > 0) {
                found += fits * arrange(aPos + 1, bEnd);
            }
        }
        MEMO[MEMOid] = found;
        return found;
    }
};

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    std::vector<SpringRow> hotTub{};
    int64_t id{0};
    SimpleParser scan{argv[1]};
    while (!scan.isEof()) {
        hotTub.emplace_back(scan, ++id);
    }

    int n = 0;
    int64_t sum = 0;
    for (auto &row : hotTub) {
        const auto value = row.arrange();
        sum += value;
        fmt::print("{:4}: {} has {} arrangements\n", ++n, row.springs, value);
    }
    fmt::print("The arrangements sum is {}\n", sum);
    fmt::print("The MEMO cache contains {} entries\n", MEMO.size());
}
