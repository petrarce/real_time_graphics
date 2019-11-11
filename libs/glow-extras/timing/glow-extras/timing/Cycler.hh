#pragma once

// https://helloacm.com/the-rdtsc-performance-timer-written-in-c/
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>

#ifdef _WIN32
#include <intrin.h>
#endif

namespace glow
{
namespace timing
{
//  Windows
#ifdef _WIN32

inline uint64_t cycles()
{
    return __rdtsc();
}

//  Linux/GCC
#else

inline uint64_t cycles()
{
    unsigned int lo, hi;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

#endif

/**
 * Usage:
 *   Cycler cycler;
 *   ... do stuff
 *   cycler.print("my stuff: ");   (also restarts cycler)
 *   ... do looped stuff
 *   cycler.print("looped action: ", n_actions)
 */
struct Cycler
{
    Cycler() : mCycles(cycles()) {}

    /// prints the number of elapsed cycles
    /// prints the number of cycles / op if ops > 0
    /// if restart, restarts the cycle counter AFTER printing the count
    void print(std::string const& prefix = "", int64_t ops = -1, bool restart = true);

    uint64_t getCycles() const { return cycles() - mCycles; }
    void restart() { mCycles = cycles(); }

private:
    uint64_t mCycles;
};
}
}
