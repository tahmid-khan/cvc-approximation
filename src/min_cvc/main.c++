// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// ReSharper disable CppTemplateArgumentsCanBeDeduced

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ios>
#include <iostream>
#include <iterator>
#include <span>

#include "backtracking.h"
#include "bitmasking.h"
#include "simple_graph.h"

/// Parses the command line.
bool parse_cmd(const std::span<const char* const> args, bool& use_bitmasking);

/// Prints a help message showing the syntax of the command line.
void show_usage(const char* const cmd_name);

/// Inputs a value of type `T` from the `is` input stream and returns it.
template<typename T> T scan(std::istream& is = std::cin);

/// Prints the elements of the `container`, separating them by spaces.
template<typename C> void output_elems(const C& container);

/// Prints the indices of the set bits (1 bits) in the binary representation of `mask`, separating
/// them by spaces.  Indexing starts at 0 on the least significant bit and increases rightwards.
void output_set_bits(std::uint64_t mask);

int main(const int argc, const char* const argv[])
{
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cin.exceptions(std::ios_base::failbit);

    bool use_bitmasking{false};
    if (const auto args = std::span(argv, argc); !parse_cmd(args, use_bitmasking)) {
        show_usage(args[0]);
        return EXIT_FAILURE;
    }

    Simple_graph g(scan<int>());
    for (auto m = scan<int>(); m-- != 0;)
        g.add_edge(scan<int>(), scan<int>());

    if (use_bitmasking) {
        const auto cvc_mask = find_cvc_by_bitmasking(g);
        output_set_bits(cvc_mask);
    }
    else {
        const auto cvc = find_cvc_by_backtracking(g);
        output_elems(cvc);
    }

    std::cout << '\n';
    return 0;
}

[[nodiscard]] bool parse_cmd(const std::span<const char* const> args, bool& use_bitmasking)
{
    use_bitmasking = args.size() == 2 && std::strcmp(args[1], "-m") == 0;
    return args.size() == 1 || use_bitmasking;
}

void show_usage(const char* const cmd_name)
{
    std::cerr << "usage: " << cmd_name << " [-m]\n";
}

template<typename T> [[nodiscard]] T scan(std::istream& is)
{
    T buf;
    is >> buf;
    return buf;
}

template<typename C> void output_elems(const C& container)
{
    if (std::empty(container)) return;
    for (auto it = std::cbegin(container);;) {
        std::cout << *it;
        if (++it == std::cend(container)) break;
        std::cout << ' ';
    }
}

void output_set_bits(std::uint64_t mask)
{
    constexpr std::uint64_t one_u64{1};
    for (int bit{0}; mask != 0; ++bit) {
        if ((mask & one_u64) != 0) {
            std::cout << bit;
            mask >>= one_u64;
            if (mask == 0) break;
            std::cout << ' ';
        }
        else mask >>= one_u64;
    }
}
