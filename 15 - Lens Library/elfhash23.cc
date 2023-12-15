#include <algorithm>
#include <cassert>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <list>
#include <ranges>
#include <string>
#include <vector>

#include "simpleparser.hpp"

using std::views::iota;

uint8_t HASH(const std::string msg) {
    uint8_t state = 0;
    for (const auto c : msg) {
        state += static_cast<uint8_t>(c);
        state *= 17;
    }
    return state;
}

void part1(const char *file, const bool debug = false) {
    int64_t sum = 0;
    SimpleParser scan{file};
    while (!scan.isEof()) {
        const std::string step = scan.getToken(',');
        scan.skipChar(',');
        const auto result = HASH(step);
        if (debug)
            fmt::print("'{}' = {}\n", step, result);
        sum += result;
    }
    fmt::print("The initialization HASH is {}\n", sum);
}

struct lens {
    std::string label;
    int64_t focal;

    auto str() const { return fmt::format("[{} {}]", label, focal); }
};

struct HASHMAP {
    std::array<std::list<lens>, 256> boxes{};

    void step(SimpleParser &scan, const bool debug = false) {
        const std::string label = scan.getAlNum();
        if (debug)
            fmt::print("\nAfter \"{}", label);
        switch (scan.peekChar()) {
        case '-':
            scan.skipChar('-');
            if (debug)
                fmt::print("-\":\n");
            remove(label);
            break;
        case '=':
            scan.skipChar('=');
            const auto focal = scan.getInt64();
            if (debug)
                fmt::print("={}\":\n", focal);
            insert(label, focal);
            break;
        }
        scan.skipChar(',');
    }

    static auto findLabel(const std::string label, auto &box) {
        for (auto it = box.begin(); it != box.end(); ++it) {
            if (it->label == label) {
                return it;
            }
        }
        return box.end();
    }

    void remove(const std::string &label) {
        auto &box = boxes[HASH(label)];
        const auto where = findLabel(label, box);
        if (where != box.end()) {
            box.erase(where);
        }
    }

    void insert(const std::string &label, int64_t focal) {
        auto &box = boxes[HASH(label)];
        const auto where = findLabel(label, box);
        if (where != box.end()) {
            where->focal = focal;
        } else {
            box.emplace_back(label, focal);
        }
    }

    int64_t sum() const {
        int64_t sum = 0;
        for (const auto box : iota(0u, boxes.size())) {
            int64_t slot = 0;
            for (const auto &lens : boxes[box]) {
                sum += (box + 1) * (++slot) * lens.focal;
            }
        }
        return sum;
    }

    void print() const {
        for (const auto n : iota(0u, boxes.size())) {
            if (boxes[n].size() > 0) {
                fmt::print("Box {}:", n);
                for (const auto &lens : boxes[n]) {
                    fmt::print(" {}", lens.str());
                }
                fmt::print("\n");
            }
        }
    }
};

void part2(const char *file, const bool debug = false) {
    HASHMAP boxes{};

    SimpleParser scan{file};
    while (!scan.isEof()) {
        boxes.step(scan, debug);
        if (debug)
            boxes.print();
    }
    fmt::print("The focusing power is {}\n", boxes.sum());
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    // selftest
    assert(HASH("HASH") == 52);
    fmt::print("HASH self verification passed\n");

    part1(argv[1]);
    part2(argv[1]);
}
