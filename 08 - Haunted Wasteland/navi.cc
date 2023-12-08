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

struct Position {
    std::string node;
    size_t step;
    auto operator<=>(const Position &other) const {
        return std::pair(node, step) <=> std::pair(other.node, other.step);
    }
    bool operator!=(const Position &other) const {
        return std::pair(node, step) != std::pair(other.node, other.step);
    }
};

// from https://fmt.dev/latest/api.html#format-api
template <> struct fmt::formatter<Position> {
    constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
    template <typename FormatContext>
    constexpr auto format(const Position &position, FormatContext &ctx) const
        -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "{}@{}", position.node, position.step);
    }
};

std::map<std::string, Node> network;
std::string directions{};

std::string networkStep(const std::string &position, size_t step) {
    switch (directions[step]) {
    case 'L':
        return network[position].left;
        break;
    case 'R':
        return network[position].right;
        break;
    default:
        fmt::print("Unknown direction\n");
        return position;
    }
}

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
        position = networkStep(position, step);
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
    size_t ghostSteps = 1;
    bool lcmIsSafe = true;

    for (auto &[name, _] : network) {
        if (name[2] != 'A') {
            continue;
        }
        auto position = Position{name, 0};
        fmt::print("starting at {}\n", position);

        const auto startingPosition = position;
        size_t step = 0;

        size_t prefixSteps = 0;
        do {
            position = Position{networkStep(position.node, step), step};
            ++step;
            if (step == directions.size()) {
                step = 0;
            }
            ++prefixSteps;
        } while (position.node[2] != 'Z');
        const auto firstTerminalPosition = position;

        size_t loopSteps = 0;
        do {
            position = Position{networkStep(position.node, step), step};
            ++step;
            if (step == directions.size()) {
                step = 0;
            }
            ++loopSteps;
            if (position.node[2] == 'Z' and position != firstTerminalPosition) {
                fmt::print("CAUTION: Possible additional terminal at {}+{}\n", prefixSteps,
                           loopSteps);
                lcmIsSafe = false;
            }
        } while (position != firstTerminalPosition);

        fmt::print("Ghost route from {} to {} took {} + {} * n steps\n", startingPosition, position,
                   prefixSteps, loopSteps);

        if (prefixSteps != loopSteps) {
            fmt::print("CAUTION: prefix steps {} != loop steps {}\n", prefixSteps, loopSteps);
            lcmIsSafe = false;
        }

        ghostSteps = std::lcm(ghostSteps, prefixSteps);
    }
    fmt::print("Your ghost took {} steps\n", ghostSteps);
    if (!lcmIsSafe) {
        printf("CAUTION: Solution might be wrong!\n");
    }
}
