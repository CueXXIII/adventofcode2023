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
    for (const auto &v : data | std::ranges::views::drop(1)) {
        fmt::print(", {}", v);
    }
    fmt::print("]\n");
}

int64_t nextValue(const Seq &data) {
    Seq delta{};
    bool allZero = true;
    for (const auto i : iota(1u, data.size())) {
        const auto value = data[i] - data[i - 1];
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
        fmt::print("Value = {}\n", value);
        expSum += value;
    }
    fmt::print("Extrapolated Sum = {}\n", expSum);
}
