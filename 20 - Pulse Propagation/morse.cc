#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <queue>
#include <ranges>
#include <string>
#include <vector>

#include "simpleparser.hpp"

using std::views::iota;
using std::views::reverse;

enum Pulse { low, high, none };

const std::string pulseName(const Pulse &p) {
    switch (p) {
    case low:
        return "low";
    case high:
        return "high";
    case none:
        return "none";
    default:
        return "INV";
    }
}

struct Module {
    std::string name; // broadcaster name is "roadcaster"
    char type;
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;

    bool stateOn{false};
    std::map<std::string, Pulse> recentPulses{};

    Module() = default;
    Module(SimpleParser &scan) {
        type = scan.getChar();
        name = scan.getToken();
        scan.skipToken("->");
        do {
            outputs.push_back(scan.getAlNum());
        } while (scan.skipChar(','));
    }

    void addInput(const std::string &in) {
        inputs.push_back(in);
        recentPulses[in] = low;
    }

    Pulse pulse(const std ::string &in, const Pulse pulse) {
        if (pulse == none) {
            fmt::print("none pulse sent to {}!", name);
            return none;
        }
        switch (type) {
        case 'b':
            return pulse;
        case '%':
            if (pulse == high) {
                return none;
            }
            stateOn = !stateOn;
            if (stateOn) {
                return high;
            } else {
                return low;
            }
        case '&':
            recentPulses[in] = pulse;
            for (const auto &[name, last] : recentPulses) {
                if (last == low) {
                    return high;
                }
            }
            return low;
        default:
            fmt::print("Invalid type {} on '{}'\n", type, name);
            return none;
        }
    }
};

struct Machine {
    std::map<std::string, Module> modules;
    std::queue<std::tuple<std::string, std::string, Pulse>> pulses;
    int64_t countLow = 0;
    int64_t countHigh = 0;

    Machine(SimpleParser &scan) {
        while (!scan.isEof()) {
            Module m{scan};
            modules[m.name] = m;
        }
        // connect inputs
        for (const auto &[name, mod] : modules) {
            for (const auto &out : modules[name].outputs) {
                if (modules.contains(out)) {
                    modules[out].addInput(name);
                } else {
                    fmt::print("Dangling connection {} -> {}\n", name, out);
                }
            }
        }
    }

    Pulse push(const bool debug = false) {
        Pulse rxAcc = none;
        // button pulse
        ++countLow;
        pulses.emplace("button", "roadcaster", low);
        if (debug) [[unlikely]] {
            fmt::print("button -low-> broadcaster\n");
        }

        while (!pulses.empty()) {
            const auto [from, here, pulse] = pulses.front();
            pulses.pop();

            if (!modules.contains(here)) {
                if (debug) [[unlikely]] {
                    fmt::print("==>{} received a {} pulse from {}\n", here, pulseName(pulse), from);
                }
                if (here == "rx") [[likely]] {
                    if (pulse == low and rxAcc == none) {
                        rxAcc = low;
                    } else {
                        rxAcc = high;
                    }
                }
            } else {
                auto &currentModule = modules[here];
                const auto next = currentModule.pulse(from, pulse);
                if (next != none) {
                    for (const auto &dest : currentModule.outputs) {
                        if (next == low) {
                            ++countLow;
                        } else if (next == high) {
                            ++countHigh;
                        }
                        pulses.emplace(here, dest, next);

                        if (debug) [[unlikely]] {
                            fmt::print("{}{} -{}-> {}\n", currentModule.type, currentModule.name,
                                       pulseName(next), dest);
                        }
                    }
                }
            }
        }
        if (debug) [[unlikely]] {
            fmt::print("\n");
        }
        return rxAcc;
    }
};

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt>\n";
        std::exit(EXIT_FAILURE);
    }

    SimpleParser scan{argv[1]};
    Machine m{scan};
    for ([[maybe_unused]] const auto n : iota(0, 1000)) {
        const auto rxVal = m.push();
        if (rxVal == low) {
            fmt::print("push {}: {}\n", n + 1, pulseName(rxVal));
        }
    }
    fmt::print("The elves calculate {} * {} = {}\n", m.countLow, m.countHigh,
               m.countLow * m.countHigh);
    return 0;
    int64_t n = 1000;
    for (;;) {
        ++n;
        const auto rxVal = m.push();
        if ((n & ((1 << 15) - 1)) == 0) {
            fmt::print("{}...\n", n);
        }
        if (rxVal == low) {
            break;
        }
    }
    fmt::print("The machine starts working after {} button presses\n", n);
}
