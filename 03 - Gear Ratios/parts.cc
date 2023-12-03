#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

#include "vec2.hpp"

using std::views::iota;

class Schema {
    std::vector<char> schematic{};
    std::vector<bool> isNumber{};
    int64_t width{};
    int64_t height{};
    char outside{'.'};

  public:
    Schema(const char *filename) {
        std::ifstream infile{filename};
        std::string line;
        while (std::getline(infile, line)) {
            width = line.size();
            height++;
            // schematic.append_range(line);
            schematic.insert(schematic.end(), line.begin(), line.end());
        }
        findSymbols();
    }

    char operator[](const Vec2<int64_t> p) const {
        if (p.x < 0 or p.x >= width or p.y < 0 or p.y >= height) {
            return '.';
        }
        return schematic.at(p.x + p.y * width);
    }
    char &operator[](const Vec2<int64_t> p) {
        if (p.x < 0 or p.x >= width or p.y < 0 or p.y >= height) {
            return outside = '.';
        }
        return schematic[p.x + p.y * width];
    }

    void findSymbols() {
        isNumber.resize(schematic.size(), false);
        for (auto y : iota(0, height)) {
            for (auto x : iota(0, width)) {
                if (std::isdigit((*this)[{x, y}])) {
                    for (const auto neigh :
                         std::initializer_list<Vec2<int64_t>>{{-1, -1},
                                                              {-1, 0},
                                                              {-1, 1},
                                                              {0, -1},
                                                              {0, 1},
                                                              {1, -1},
                                                              {1, 0},
                                                              {1, 1}}) {
                        const auto c = (*this)[Vec2<int64_t>{x, y} + neigh];
                        if (c != '.' and !std::isdigit(c)) {
                            isNumber[x + y * width] = true;
                            // expand number to left/right
                            for (auto d = -1; std::isdigit((*this)[{x - d, y}]);
                                 --d) {
                                isNumber[x - d + y * width] = true;
                            }
                            for (auto d = 1; std::isdigit((*this)[{x - d, y}]);
                                 ++d) {
                                isNumber[x - d + y * width] = true;
                            }
                        }
                    }
                }
            }
        }
    }

    void sumNumbers() const {
        int64_t sum = 0;
        for (auto y : iota(0, height)) {
            for (auto x = 0; x < width; ++x) {
                int64_t current = 0;
                while (isNumber[x + y * width] and x < width) {
                    current *= 10;
                    current += (*this)[{x, y}] - '0';
                    ++x;
                }
                if (current != 0) {
                }
                sum += current;
            }
        }
        fmt::print("The engine parts sum to {}\n", sum);
    }

    void print() const {
        for (auto y : iota(0, height)) {
            for (auto x : iota(0, width)) {
                if (isNumber[x + y * width]) {
                    std::cout << (*this)[{x, y}];
                } else {
                    std::cout << ' ';
                }
            }
            std::cout << '\n';
        }
        std::cout << '\n';
    }
};

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    Schema broken{argv[1]};
    // broken.print();
    broken.sumNumbers();
}
