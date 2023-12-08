#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <map>
#include <ranges>
#include <string>
#include <vector>

#include "simpleparser.hpp"

using std::views::iota;

struct Node {
    std::string name;
    std::string left;
    std::string right;
    auto operator<=>(const Node &other) const { return name <=> other.name; }
};

std::map<std::string, Node> network;

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    SimpleParser scanner{argv[1]};
    const auto directions = scanner.getLine();
    while (!scanner.isEof()) {
        const auto name = scanner.getAlNum();
        scanner.skipToken("= (");
        const auto left = scanner.getAlNum();
        scanner.skipChar(',');
        const auto right = scanner.getAlNum();
        scanner.skipChar(')');
        network.emplace(name, Node{name, left, right});
    }

    size_t totalSteps = 0;
    std::string position = "AAA";
    size_t step = 0;
    fmt::print("step {} position {}\n", step, position);
    while (position != "ZZZ") {
        switch (directions[step]) {
        case 'L':
            position = network[position].left;
            break;
        case 'R':
            position = network[position].right;
            break;
        default:
            fmt::print("Unknown direction\n");
            break;
        }
        ++step;
        if (step == directions.size()) {
            step = 0;
        }
        ++totalSteps;
        fmt::print("step {} position {}\n", step, position);
    }
    fmt::print("You took {} steps\n", totalSteps);
}
