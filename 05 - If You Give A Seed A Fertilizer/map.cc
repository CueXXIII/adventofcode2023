#include <algorithm>
#include <cctype>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <limits>
#include <ranges>
#include <string>
#include <vector>

#include "simpleparser.hpp"

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
};

using std::views::iota;

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    std::vector<Mapper> mappings{};
    SimpleParser scanner{argv[1]};

    std::vector<int64_t> seeds{};
    scanner.skipToken("seeds:");
    while (!scanner.isEof() and std::isdigit(scanner.peekChar())) {
        seeds.emplace_back(scanner.getInt64());
    }

    while (!scanner.isEof()) {
        auto title = scanner.getToken();
        scanner.getLine();
        mappings.emplace_back(title, scanner);
        std::cout << title << '\n';
    }
    int64_t min_location = std::numeric_limits<int64_t>::max();
    for (int64_t location : seeds) {
        for (const auto &mapper : mappings) {
            location = mapper.map(location);
        }
        fmt::print("Seed mapped to {}\n", location);
        min_location = std::min(min_location, location);
    }
    fmt::print("Lowest location is {}\n", min_location);
}
