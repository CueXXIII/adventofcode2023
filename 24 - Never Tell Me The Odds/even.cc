#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

#include "simpleparser.hpp"
#include "vec3.hpp"

using std::views::iota;
using std::views::reverse;

struct Hailstone {
    Vec3l pos{};
    Vec3l dir{};

    Hailstone(SimpleParser &scan) {
        const auto xp = scan.getInt64();
        scan.skipChar(',');
        const auto yp = scan.getInt64();
        scan.skipChar(',');
        const auto zp = scan.getInt64();
        scan.skipChar('@');
        const auto xd = scan.getInt64();
        scan.skipChar(',');
        const auto yd = scan.getInt64();
        scan.skipChar(',');
        const auto zd = scan.getInt64();
        pos = {xp, yp, zp};
        dir = {xd, yd, zd};
    }

    static Vec2<double> dVec(const Vec3l &v) {
        return {static_cast<double>(v.x), static_cast<double>(v.y)};
    }
    bool intersectXY(const Hailstone other, bool isExample) const {
        if (isExample) {
            fmt::print("Hailstone A: {} @ {}\n", pos, dir);
            fmt::print("Hailstone B: {} @ {}\n", other.pos, other.dir);
        }
        const auto m1 = dVec(dir);
        const auto b1 = dVec(pos);
        const auto m2 = dVec(other.dir);
        const auto b2 = dVec(other.pos);

        const auto div = m1.y * m2.x - m1.x * m2.y;
        if (div == 0) {
            if (isExample) {
                fmt::print("Hailstones' paths are parallel; they never intersect.\n\n");
            }
            return false;
        }
        const auto t2 = (m1.x * (b2.y - b1.y) - m1.y * (b2.x - b1.x)) / div;
        const auto t1 = (m2.x * (b1.y - b2.y) - m2.y * (b1.x - b2.x)) / (div * -1);
        if (t2 < 0) {
            if (isExample) {
                if (t1 < 0) {
                    fmt::print("Hailstones' paths crossed in the past for both hailstones.\n\n");
                } else {
                    fmt::print("Hailstones' paths crossed in the past for hailstone B.\n\n");
                }
            }
            return false;
        }
        if (t1 < 0) {
            if (isExample) {
                fmt::print("Hailstones' paths crossed in the past for hailstone A.\n\n");
            }
            return false;
        }
        const auto i = m2 * t2 + b2;
        bool inside;
        if (isExample) {
            inside = (i.x >= 7 and i.x <= 27 and i.y >= 7 and i.y <= 27);
            fmt::print("Hailstones' paths will cross {} the test area (at {}).\n\n",
                       inside ? "inside" : "outside", i);
        } else {
            inside = (i.x >= 200000000000000 and i.x <= 400000000000000 and
                      i.y >= 200000000000000 and i.y <= 400000000000000);
        }
        return inside;
    }
};

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    std::vector<Hailstone> weather{};
    SimpleParser scan{argv[1]};
    while (!scan.isEof()) {
        weather.emplace_back(scan);
    }

    int64_t count1 = 0;
    for (const auto i : iota(0u, weather.size())) {
        for (const auto j : iota(i + 1, weather.size())) {
            if (weather[i].intersectXY(weather[j], argv[1][0] == 'e')) {
                ++count1;
            }
        }
    }
    fmt::print("{} intersections occur within the test area\n\n", count1);

    fmt::print("sage:\n");
    fmt::print("t0, t1, t2, mRx, mRy, mRz, bRx, bRy, bRz, answer = "
               "var('t0', 't1', 't2', 'mRx', 'mRy', 'mRz', 'bRx', 'bRy', 'bRz', 'answer')\n");
    fmt::print("solve([");
    for (const auto idx : iota(0, 3)) {
        const auto &hail = weather[idx];
        fmt::print("{}*t{}+{} == mRx*t{}+bRx, ", hail.dir.x, idx, hail.pos.x, idx);
        fmt::print("{}*t{}+{} == mRy*t{}+bRy, ", hail.dir.y, idx, hail.pos.y, idx);
        fmt::print("{}*t{}+{} == mRz*t{}+bRz, ", hail.dir.z, idx, hail.pos.z, idx);
    }
    fmt::print("answer == bRx+bRy+bRz], t0, t1, t2, mRx, mRy, mRz, bRx, bRy, bRz, answer)\n");
}
