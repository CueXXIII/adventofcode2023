#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <map>
#include <ranges>
#include <set>
#include <string>
#include <vector>

#include "vec2.hpp"

using std::views::iota;

class Schema {
    std::vector<char> schematic{};
    std::vector<int64_t> numberId{};
    std::map<int64_t, int64_t> foundNumbers{};
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
        numberId.resize(schematic.size(), 0);
        int64_t freeId = 0;
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
                            if (numberId[x + y * width] == 0) {
                                ++freeId;
                                numberId[x + y * width] = freeId;
                                // expand number to left/right
                                for (auto d = -1;
                                     std::isdigit((*this)[{x - d, y}]); --d) {
                                    numberId[x - d + y * width] = freeId;
                                }
                                for (auto d = 1;
                                     std::isdigit((*this)[{x - d, y}]); ++d) {
                                    numberId[x - d + y * width] = freeId;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    void sumNumbers() {
        int64_t sum = 0;
        for (auto y : iota(0, height)) {
            for (auto x = 0; x < width; ++x) {
                int64_t current = 0;
                const auto currentId = numberId[x + y * width];
                auto posId = currentId;
                while (posId > 0) {
                    current *= 10;
                    current += (*this)[{x, y}] - '0';
                    ++x;
                    if (x == width) {
                        break;
                    }
                    posId = numberId[x + y * width];
                }
                if (current != 0) {
                    foundNumbers[currentId] = current;
                }
                sum += current;
            }
        }
        fmt::print("The engine parts sum to {}\n", sum);
    }

    void sumGears() const {
        int64_t ratioSum = 0;
        for (auto y : iota(0, height)) {
            for (auto x : iota(0, width)) {
                if ((*this)[{x, y}] == '*') {
                    std::set<int64_t> adjNumId{};
                    for (const auto neigh :
                         std::initializer_list<Vec2<int64_t>>{{-1, -1},
                                                              {-1, 0},
                                                              {-1, 1},
                                                              {0, -1},
                                                              {0, 1},
                                                              {1, -1},
                                                              {1, 0},
                                                              {1, 1}}) {
                        const auto num = neigh + Vec2<int64_t>{x, y};
                        const auto c = (*this)[num];
                        if (std::isdigit(c)) {
                            adjNumId.insert(numberId[num.x + num.y * width]);
                        }
                    }
                    if (adjNumId.size() == 2) {
                        int64_t ratio = 1;
                        for (const auto r : adjNumId) {
                            ratio *= foundNumbers.at(r);
                        }
                        ratioSum += ratio;
                    }
                }
            }
        }
        fmt::print("The gear ratios sum to {}\n", ratioSum);
    }

    void print() const {
        for (auto y : iota(0, height)) {
            for (auto x : iota(0, width)) {
                if (numberId[x + y * width] > 0) {
                    std::cout << (*this)[{x, y}];
                } else if ((*this)[{x, y}] == '*') {
                    std::cout << '*';
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
    broken.sumGears();
}
