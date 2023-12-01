#pragma once

#include <chrono>

// usage:
// const auto t1 = timeNow()
//   - gets current point in time
// timeDiff(t1, t2)
//   - returns the diff between 2 time points in seconds

inline auto timeNow() { return std::chrono::high_resolution_clock::now(); }

inline auto timeDiff(const auto &from, const auto &to) {
    return std::chrono::duration_cast<std::chrono::duration<double>>(to - from)
        .count();
}
