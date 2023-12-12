// Forward Spring Correction
#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

#include "simpleparser.hpp"

using std::views::iota;

struct SpringRow {
    std::vector<char> springs;
    std::vector<int64_t> broken;
    std::string springsStr;

    SpringRow(SimpleParser &scan) {
        springsStr = scan.getToken();
        springsStr =
            springsStr + '?' + springsStr + '?' + springsStr + '?' + springsStr + '?' + springsStr;
        springs.insert(springs.end(), springsStr.begin(), springsStr.end());

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

    void skipSpace(int64_t &x, bool skipUnknown = false) const {
        while (x < springs.size() and (springs[x] == '.' or (skipUnknown and springs[x] == '?'))) {
            ++x;
        }
    }

    bool canBeSpring(const int64_t x, const int64_t len) const {
        for (const auto pos : iota(x, x + len)) {
            if (pos == springs.size()) {
                return false;
            }
            if (springs[pos] == '.') {
                return false;
            }
        }
        if (x + len == springs.size() or springs[x + len] != '#') {
            return true;
        } else {
            return false;
        }
    }

    std::string fillSprings(const std::string &orig, const int64_t pos, const int64_t len) const {
        auto filled = orig;
        for (const auto n : iota(pos, pos + len)) {
            filled[n] = '#';
        }
        return filled;
    }

    // x: position in springs where to start looking
    // n: position in broken   "
    // arrange() is always called after a spring was complete
    int64_t arrange(int64_t x = 0, const int64_t n = 0) const {
        if (n == broken.size()) {
            skipSpace(x, true);
            if (x >= springs.size()) {
                return 1;
            } else {
                return 0;
            }
        }

        int64_t found = 0;
        for (;;) {
            if (x >= springs.size()) {
                break;
            }
            if (canBeSpring(x, broken[n])) {
                if (x + broken[n] == springs.size() or springs[x + broken[n]] != '#') {
                    found += arrange(x + broken[n] + 1, n + 1);
                }
            }
            if (springs[x] == '#') {
                break;
            }
            ++x;
        };
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
        fmt::print("{:4}: {} has {} arrangements\n", ++n, row.springsStr, value);
    }
    fmt::print("The arrangements sum is {}\n", sum);
}
