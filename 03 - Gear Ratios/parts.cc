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

template <typename T> struct Mapping {
    std::vector<T> data{};
    T outside{};
    int64_t width{};
    int64_t height{};
    const T empty{};

    inline bool validPos(const Vec2<int64_t> &pos) const {
        return validPos(pos.x, pos.y);
    }
    inline bool validPos(const int64_t x, const int64_t y) const {
        return x >= 0 and y >= 0 and x < height and y < height;
    }
    inline size_t index(const Vec2<int64_t> &pos) const {
        return index(pos.x, pos.y);
    }
    inline size_t index(const int64_t x, const int64_t y) const {
        return static_cast<size_t>(x + y * width);
    }

    Mapping() = default;
    Mapping(const char *filename, T empty = {}) : empty(empty) {
        std::ifstream infile{filename};
        std::string line;
        while (std::getline(infile, line)) {
            width = line.size();
            height++;
            // data.append_range(line);
            data.insert(data.end(), line.begin(), line.end());
        }
    }
    Mapping(int64_t width, int64_t height, T value = {}, T empty = {})
        : width(width), height(height), empty(empty) {
        data.resize(width * height, value);
    }

    // access by [x, y]
    T operator[](const int64_t x, const int64_t y) const {
        if (!validPos(x, y)) {
            return empty;
        }
        return data[index(x, y)];
    }
    T &operator[](const int64_t x, const int64_t y) {
        if (!validPos(x, y)) {
            return outside = empty;
        }
        return data[index(x, y)];
    }
    // access by [Vec2]
    T operator[](const Vec2<int64_t> p) const {
        if (!validPos(p)) {
            return empty;
        }
        return data[index(p)];
    }
    T &operator[](const Vec2<int64_t> p) {
        if (!validPos(p)) {
            return outside = empty;
        }
        return data[index(p)];
    }
};

constexpr std::array<Vec2<int64_t>, 4> neighbours4{
    {{-1, 0}, {0, -1}, {0, 1}, {1, 0}}};

constexpr std::array<Vec2<int64_t>, 8> neighbours8{
    {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}}};

struct Schema {
    Mapping<char> schematic{};
    int64_t width{};
    int64_t height{};

    Mapping<int64_t> numberId{};
    std::map<int64_t, int64_t> foundNumbers{};

    Schema(const char *filename)
        : schematic(filename, '.'), width(schematic.width),
          height(schematic.height), numberId{width, height} {
        findSymbols();
    }

    void findSymbols() {
        int64_t freeId = 0;
        for (auto y : iota(0, height)) {
            for (auto x : iota(0, width)) {
                const Vec2<int64_t> pos{x, y};
                if (std::isdigit(schematic[pos])) {
                    for (const auto neigh : neighbours8) {
                        const auto c = schematic[pos + neigh];
                        if (c != '.' and !std::isdigit(c)) {
                            if (numberId[pos] == 0) {
                                ++freeId;
                                numberId[pos] = freeId;
                                // expand number to left/right
                                for (auto d = -1;
                                     std::isdigit(schematic[x + d, y]); --d) {
                                    numberId[x + d, y] = freeId;
                                }
                                for (auto d = 1;
                                     std::isdigit(schematic[x + d, y]); ++d) {
                                    numberId[x + d, y] = freeId;
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
                const auto currentId = numberId[x, y];
                auto posId = currentId;
                while (posId > 0) {
                    current *= 10;
                    current += schematic[x, y] - '0';
                    ++x;
                    posId = numberId[x, y];
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
                const Vec2<int64_t> pos{x, y};
                if (schematic[pos] == '*') {
                    std::set<int64_t> adjNumId{};
                    for (const auto neigh : neighbours8) {
                        const auto num = pos + neigh;
                        const auto c = schematic[num];
                        if (std::isdigit(c)) {
                            adjNumId.insert(numberId[num]);
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
                if (numberId[x, y] > 0) {
                    std::cout << schematic[x, y];
                } else if (schematic[x, y] == '*') {
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
