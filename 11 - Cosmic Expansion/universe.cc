#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

#include "grid.hpp"
#include "vec2.hpp"

using std::views::iota;

int64_t distSum(auto &positions, const int64_t expansion = 2) {
    int64_t sum = 0;
    int64_t openDist = 0;
    auto it = positions.begin();
    auto current = *it;
    int64_t count = 1;

    while (++it != positions.end()) {
        const auto next = *it;

        if (next > current) {
            const auto distToPrev = (next - current - 1) * expansion + 1;
            openDist += count * distToPrev;
        }
        sum += openDist;
        ++count;
        current = next;
    }
    return sum;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    std::vector<int64_t> x;
    std::vector<int64_t> y;

    std::ifstream infile{argv[1]};
    std::string line;
    int64_t yPos = 0;
    while (std::getline(infile, line)) {
        for (auto xPos = line.find('#', 0); xPos != std::string::npos;
             xPos = line.find('#', xPos + 1)) {
            x.push_back(xPos);
            y.push_back(yPos);
        }
        ++yPos;
    }
    std::ranges::sort(x);
    std::ranges::sort(y);

    for (const auto e : {2, 10, 100, 1'000'000}) {
        const auto xDistSum = distSum(x, e);
        const auto yDistSum = distSum(y, e);
        fmt::print("The x{} sum of all paths is {}\n", e, xDistSum + yDistSum);
    }
}
