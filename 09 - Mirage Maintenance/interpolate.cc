#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

#include "simpleparser.hpp"

using std::views::iota;
using Seq = std::vector<int64_t>;

std::vector<Seq> OASIS{};

Seq readDataLine(SimpleParser &scan) {
    Seq data{};
    data.push_back(scan.getInt64());
    while (!scan.isEol()) {
        data.push_back(scan.getInt64());
    }
    return data;
}

void print(const Seq &data) {
    if (data.size() == 0) {
        fmt::print("[]\n");
        return;
    }
    fmt::print("[{}", data[0]);
    for (const auto &value : data | std::views::drop(1)) {
        fmt::print(", {}", value);
    }
    fmt::print("]\n");
}

int64_t nextValue(const Seq &data) {
    Seq delta{};
    bool allZero = true;
    for (const auto &value : data | std::views::adjacent_transform<2>(
                                        [](const auto n0, const auto n1) { return n1 - n0; })) {
        if (value != 0) {
            allZero = false;
        }
        delta.push_back(value);
    }
    if (allZero) {
        return data[0];
    } else {
        const auto value = data.back() + nextValue(delta);
        return value;
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    SimpleParser scan{argv[1]};
    while (!scan.isEof()) {
        OASIS.push_back(readDataLine(scan));
    }

    int64_t expSum = 0;
    for (const auto &seq : OASIS) {
        const auto value = nextValue(seq);
        expSum += value;
    }
    fmt::print("Extrapolated next sum = {}\n", expSum);

    expSum = 0;
    for (auto &seq : OASIS) {
        std::reverse(seq.begin(), seq.end());
        const auto value = nextValue(seq);
        expSum += value;
    }
    fmt::print("Extrapolated prev sum = {}\n", expSum);
}
