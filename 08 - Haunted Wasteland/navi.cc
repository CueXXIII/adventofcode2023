#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
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
std::string directions{};

void navi1();
void navi2();

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    SimpleParser scanner{argv[1]};
    directions = scanner.getLine();
    while (!scanner.isEof()) {
        const auto name = scanner.getAlNum();
        scanner.skipToken("= (");
        const auto left = scanner.getAlNum();
        scanner.skipChar(',');
        const auto right = scanner.getAlNum();
        scanner.skipChar(')');
        network.emplace(name, Node{name, left, right});
    }
    navi1();
    fmt::print("\n");
    navi2();
}

void navi1() {
    size_t totalSteps = 0;
    std::string position = "AAA";
    size_t step = 0;

    fmt::print("step {} position {}\n", step, position);

    if (!network.contains(position)) {
        fmt::print("Bad input for part 1\n");
        return;
    }

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

void navi2() {
    std::vector<std::string> positions{};
    for (const auto &[name, node] : network) {
        if (name[2] == 'A') {
            positions.emplace_back(name);
            fmt::print("starting at {}\n", name);
        }
    }

    size_t ghostSteps = 1;

    for (auto position : positions) {
        size_t totalSteps = 0;
        const auto starting_position = position;
        size_t step = 0;
        while (position[2] != 'Z') {
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
        }
        fmt::print("Ghost route from {} to {} took {} steps\n", starting_position, position,
                   totalSteps);
        ghostSteps = std::lcm(ghostSteps, totalSteps);
    }
    fmt::print("Your ghost took {} steps\n", ghostSteps);
}
