// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <ios>
#include <iostream>
#include <set>
#include <utility>
#include <vector>

#include "simple_graph.h"

// ReSharper disable CppTemplateArgumentsCanBeDeduced

template<typename T> T scan(std::istream& is = std::cin)
{
    T buf;
    is >> buf;
    return buf;
}

/// Checks if the `selection` of vertices forms a connected vertex cover of `g`.
bool selection_is_cvc(const Simple_graph& g, std::uint64_t mask)
{
    std::vector<int> selection;
    std::vector<bool> is_selected(g.order(), false);

    // check if all edges are covered
    std::set<std::pair<int, int>> covered;
    for (int v{0}; mask != 0; mask >>= 1U, ++v) {
        if ((mask & UINT64_C(1)) != 0) {
            selection.push_back(v);
            is_selected[v] = true;
            for (const auto u : g.neighbors(v))
                covered.emplace(std::min(u, v), std::max(u, v));
        }
    }
    if (static_cast<int>(covered.size()) != g.size()) return false;

    std::vector<bool> is_visited(g.order(), false);

    // visit all vertices connected to root
    const std::function<void(int)> visit_connected = [&](const int v) {
        is_visited[v] = true;
        for (const auto u : g.neighbors(v))
            if (is_selected[u] && !is_visited[u]) visit_connected(u);
    };
    const int root{*std::ranges::min_element(selection)};
    visit_connected(root);

    // if any vertex in selection is not visited, it's not connected to root and
    // hence the selection of vertices is not connected
    return std::ranges::all_of(selection, [&](const int v) { return is_visited[v]; });
}

constexpr std::uint64_t next_selection(unsigned n, std::uint64_t mask)
{
    if (mask == 0) return 1;
    if (mask == (UINT64_C(1) << n) - 1) return 0;
    const auto initial_zeros = static_cast<unsigned>(std::countr_zero(mask));
    const auto subsequent_ones = static_cast<unsigned>(std::countr_one(mask >> initial_zeros));
    const unsigned total{initial_zeros + subsequent_ones};
    const std::uint64_t m{UINT64_C(1) << static_cast<unsigned>(total)};
    mask &= ~(m - 1);
    if (total == n) mask |= (UINT64_C(1) << (subsequent_ones + 1)) - 1;
    else {
        mask |= (UINT64_C(1) << (subsequent_ones - 1)) - 1;
        mask |= m;
    }
    return mask;
}

void print_set_bits(std::uint64_t mask)
{
    int bit{0};
    for (;;) {
        if ((mask & UINT64_C(1)) != 0) {
            std::cout << bit;
            mask >>= UINT64_C(1);
            if (mask == 0) {
                std::cout << '\n';
                return;
            }
            std::cout << ' ';
        }
        else mask >>= UINT64_C(1);
        ++bit;
    }
}

int main(const int argc, const char* const argv[])
{
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cin.exceptions(std::ios_base::failbit);

    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if (argc > 2 || (argc == 2 && strcmp(argv[1], "-c") != 0)) {
        std::cerr << "Usage: " << argv[0] << " [-c]\n";
        return EXIT_FAILURE;
    }
    // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    /// output mode:
    ///   - false => print the vertices in the CVC
    ///   - true => print the cardinality of the CVC
    const bool out_mode{argc == 2};

    Simple_graph g(scan<int>());
    for (auto m = scan<int>(); m-- != 0;)
        g.add_edge(scan<int>(), scan<int>());

    std::uint64_t selection{1};

    do { // NOLINT(cppcoreguidelines-avoid-do-while)
        if (selection_is_cvc(g, selection)) {
            if (out_mode) std::cout << std::popcount(selection);
            else print_set_bits(selection);
            std::cout << '\n';
            break;
        }
        selection = next_selection(static_cast<unsigned>(g.order()), selection);
    } while (selection != 0);

    return 0;
}
