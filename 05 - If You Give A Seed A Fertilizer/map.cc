#include <algorithm>
#include <cctype>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <limits>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

#include "simpleparser.hpp"

using std::views::iota;
using LocationRange = std::pair<int64_t, int64_t>;

struct Mapper {
    struct Mapping {
        int64_t dst;
        int64_t src;
        int64_t len;
        auto operator<=>(const Mapping &other) const { return src <=> other.src; }
    };
    const std::string title;
    std::vector<Mapping> mapping{};

    Mapper(const std::string &title, SimpleParser &infile) : title(title) {
        while (!infile.isEof() and std::isdigit(infile.peekChar())) {
            const auto dst = infile.getInt64();
            const auto src = infile.getInt64();
            const auto len = infile.getInt64();
            mapping.emplace_back(dst, src, len);
        }
        std::sort(mapping.begin(), mapping.end());
    }

    int64_t map(const int64_t from) const {
        for (const auto &[d, s, l] : mapping) {
            if (from >= s and from < s + l) {
                return from - s + d;
            }
        }
        return from;
    }

    // range may include location
    std::optional<Mapping> rangeLeft(const int64_t location) const {
        if (mapping.front().src > location) {
            return {};
        }
        for (const auto i : iota(1u, mapping.size())) {
            if (mapping[i].src > location) {
                return mapping[i - 1];
            }
        }
        return mapping.back();
    }
    // range after location
    std::optional<Mapping> rangeRight(const int64_t location) const {
        for (const auto &map : mapping) {
            if (map.src > location) {
                return map;
            }
        }
        return {};
    }

    std::vector<LocationRange> map(const std::vector<LocationRange> &from) const {
        std::vector<LocationRange> to{};
        for (const auto &srcRange : from) {
            auto currentSrc = srcRange.first;
            while (currentSrc < srcRange.first + srcRange.second) {
                const auto currentMap = rangeLeft(currentSrc);
                const auto srcLenLeft = srcRange.second - (currentSrc - srcRange.first);
                if (currentMap && currentSrc < currentMap->src + currentMap->len) {
                    // case 1: currentSrc in mapping
                    const auto dstLenLeft = currentMap->len - (currentSrc - currentMap->src);
                    const auto len = std::min(srcLenLeft, dstLenLeft);
                    to.emplace_back(currentSrc - currentMap->src + currentMap->dst, len);
                    currentSrc += len;
                } else {
                    // case 2a: currentSrc after mapping
                    // case 2b: currentSrc left of mappings
                    const auto nextMap = rangeRight(currentSrc);
                    if (nextMap) {
                        // case 1: we have a next mapping
                        const auto dstLenLeft = nextMap->src - currentSrc;
                        const auto len = std::min(srcLenLeft, dstLenLeft);
                        to.emplace_back(currentSrc, len);
                        currentSrc += len;
                    } else {
                        // case 2: no next mapping
                        to.emplace_back(currentSrc, srcLenLeft);
                        currentSrc += srcLenLeft;
                    }
                }
            }
        }
        std::sort(to.begin(), to.end());
        return to;
    }
};

int64_t minSeed(const std::vector<LocationRange> &locations) { return locations.front().first; }

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    std::vector<Mapper> mappings{};
    SimpleParser scanner{argv[1]};

    std::vector<int64_t> seeds{};
    std::vector<LocationRange> seedPairs{};
    scanner.skipToken("seeds:");
    while (!scanner.isEof() and std::isdigit(scanner.peekChar())) {
        const auto seedStart = scanner.getInt64();
        const auto seedLen = scanner.getInt64();
        seeds.emplace_back(seedStart);
        seeds.emplace_back(seedLen);
        seedPairs.emplace_back(seedStart, seedLen);
    }

    while (!scanner.isEof()) {
        auto title = scanner.getToken();
        scanner.getLine();
        mappings.emplace_back(title, scanner);
    }

    int64_t min_location = std::numeric_limits<int64_t>::max();
    for (auto location : seeds) {
        for (const auto &mapper : mappings) {
            location = mapper.map(location);
        }
        min_location = std::min(min_location, location);
    }
    fmt::print("Lowest location for {} seeds is {}\n", seeds.size(), min_location);

    min_location = std::numeric_limits<int64_t>::max();
    for (auto location : seedPairs) {
        std::vector<LocationRange> locations{location};
        for (const auto &mapper : mappings) {
            locations = mapper.map(locations);
        }
        min_location = std::min(min_location, minSeed(locations));
    }
    fmt::print("Lowest location for all seeds is {}\n", min_location);
}
