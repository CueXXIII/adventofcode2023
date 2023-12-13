// Forward Spring Correction V7
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

std::unordered_map<std::string, int64_t> MEMO1{};
std::unordered_map<std::string, int64_t> MEMO2{};

struct Area {
    int64_t len = 0;
    std::string layout{};
    bool hasSpring = false;
};

template <size_t Dups> struct SpringRow {
    std::vector<Area> areas{};
    std::vector<int64_t> broken{};
    std::string springs{};
    int64_t id{};

    SpringRow(SimpleParser &scan, const int64_t id) : id(id) {
        const auto inSprings = scan.getToken();
        springs = inSprings;
        for ([[maybe_unused]] const auto _ : iota(1u, Dups)) {
            springs += '?';
            springs += inSprings;
        }

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
        for ([[maybe_unused]] const auto _ : iota(0u, Dups)) {
            broken.insert(broken.end(), bpart.begin(), bpart.end());
        }
    }

    static int64_t canSkip(const std::string_view &layout, const size_t size) {
        const auto result = !layout.substr(0u, size).contains('#');
        return result;
    }

    static int64_t canBeBroken(const std::string_view &layout, const size_t size) {
        const auto result = layout.size() >= size;
        return result;
    }

    int64_t countFits(const std::string_view &layout, const size_t bStart,
                      const size_t bEnd) const {
        std::string MEMOid{layout};
        for (const auto bPos : iota(bStart, bEnd)) {
            MEMOid += fmt::format(":{}", broken[bPos]);
        }

        if (MEMO1.contains(MEMOid)) {
            return MEMO1[MEMOid];
        }

        const auto result = countFits_(layout, bStart, bEnd);
        MEMO1[MEMOid] = result;
        return result;
    }

    int64_t countFits_(const std::string_view &layout, const size_t bStart,
                       const size_t bEnd) const {
        if (bStart == bEnd) {
            if (canSkip(layout, layout.size())) {
                return 1;
            } else {
                return 0;
            }
        }

        int64_t sum = 0;
        for (size_t aPos = 0; aPos < layout.size(); ++aPos) {
            if (canSkip(layout, aPos)) {
                if (canBeBroken(layout.substr(aPos), broken[bStart])) {
                    if (layout.size() == aPos + broken[bStart]) {
                        if (bStart + 1 == bEnd) {
                            sum += 1;
                        }
                    } else {
                        const auto layoutBehind = layout.substr(aPos + broken[bStart]);
                        if (canSkip(layoutBehind, 1)) {
                            sum += countFits(layoutBehind.substr(1), bStart + 1, bEnd);
                        }
                    }
                }
            }
        }
        return sum;
    }

    // aPos: position in areas that can contains springs
    // bPos: position in broken number of springs
    // arrange() is always called after a spring was complete
    int64_t arrange(const size_t aPos = 0, const size_t bPos = 0) const {
        const std::string MEMOid{fmt::format("{};{};{}", id, aPos, bPos)};
        if (MEMO2.contains(MEMOid)) {
            return MEMO2[MEMOid];
        }

        const auto result = arrange_(aPos, bPos);
        MEMO2[MEMOid] = result;
        return result;
    }

    int64_t arrange_(const size_t aPos = 0, const size_t bPos = 0) const {
        if (aPos == areas.size()) {
            if (bPos == broken.size()) {
                return 1;
            } else {
                return 0;
            }
        }

        int64_t found = 0;
        for (const auto bEnd : iota(bPos, broken.size() + 1)) {
            const auto fits = countFits(areas[aPos].layout, bPos, bEnd);
            if (fits > 0) {
                found += fits * arrange(aPos + 1, bEnd);
            }
        }
        return found;
    }
};

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    std::vector<SpringRow<1>> coldTub{};
    std::vector<SpringRow<5>> hotTub{};

    {
        int64_t id{0};
        SimpleParser scan{argv[1]};
        while (!scan.isEof()) {
            coldTub.emplace_back(scan, ++id);
        }
    }
    {
        int64_t id{0};
        SimpleParser scan{argv[1]};
        while (!scan.isEof()) {
            hotTub.emplace_back(scan, ++id);
        }
    }

    int n = 0;
    int64_t sumFold = 0;
    int64_t sumUnfold = 0;
    for (auto row : iota(0u, hotTub.size())) {
        const auto valueFold = coldTub[row].arrange();
        MEMO2.clear();
        const auto valueUnfold = hotTub[row].arrange();
        sumFold += valueFold;
        sumUnfold += valueUnfold;
        fmt::print("{:4}: {} has {} / {} arrangements\n", ++n, coldTub[row].springs, valueFold,
                   valueUnfold);
        MEMO1.clear();
        MEMO2.clear();
    }
    fmt::print("The arrangements sum is {}\n", sumFold);
    fmt::print("The unfolded arrangements sum is {}\n", sumUnfold);
}
