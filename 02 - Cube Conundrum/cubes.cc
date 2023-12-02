#include <algorithm>
#include <cctype>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

#include "simpleparser.hpp"
#include "vec3.hpp"

using std::views::iota;

const auto RedMax = 12;
const auto GreenMax = 13;
const auto BlueMax = 14;

static constexpr auto Max = Vec3(12, 13, 14);

struct Game {
    int64_t id;
    std::vector<Vec3<int64_t>> sets;
};

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    std::vector<Game> games{};
    int64_t validIdSum = 0;

    SimpleParser scanner{argv[1]};
    while (!scanner.isEof()) {
        Game current{};
        bool validGame = true;

        scanner.skipToken("Game");
        current.id = scanner.getInt64();
        scanner.skipChar(':');
        while (std::isdigit(scanner.peekChar())) {
            Vec3<int64_t> set{};

            while (std::isdigit(scanner.peekChar())) {
                const auto amount = scanner.getInt64();
                // read color name + /[,;]*/
                const auto color = scanner.getToken();
                switch (color[0]) {
                case 'r':
                    set.x += amount;
                    break;
                case 'g':
                    set.y += amount;
                    break;
                case 'b':
                    set.z += amount;
                    break;
                default:
                    fmt::print("Unknown color {} in id {}\n", color,
                               current.id);
                    break;
                }
                // set continues with ,
                if (color[color.size() - 1] != ',') {
                    break;
                }
            }
            current.sets.emplace_back(set);
            if (set.x > Max.x or set.y > Max.y or set.z > Max.z) {
                validGame = false;
            }
        }
        games.emplace_back(current);
        if (validGame) {
            validIdSum += current.id;
        }
    }
    fmt::print("The valid GameIDs sum to {}\n", validIdSum);
}
