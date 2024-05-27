#include "bitmasking.h"

#include <algorithm>
#include <bit>
#include <cstdint>
#include <functional>
#include <iterator>
#include <set>
#include <vector>

constexpr std::uint64_t one_u64{1};

/// Checks if the `selection` of vertices forms a connected vertex cover of `g`.
bool selection_is_cvc(const Simple_graph& g, std::uint64_t mask)
{
    std::vector<int> selection;
    std::vector<bool> is_selected(g.order(), false);

    // check if all edges are covered
    std::set<std::pair<int, int>> covered;
    for (int v{0}; mask != 0; mask >>= 1U, ++v) {
        if ((mask & one_u64) != 0) {
            selection.push_back(v);
            is_selected[v] = true;
            for (const auto u : g.neighbors(v))
                covered.emplace(std::min(u, v), std::max(u, v));
        }
    }
    if (std::ssize(covered) != g.size()) return false;

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
    return std::ranges::all_of(selection, [&is_visited](const int v) { return is_visited[v]; });
}

constexpr std::uint64_t next_selection(unsigned n, std::uint64_t mask)
{
    if (mask == 0) return 1;
    if (mask == (one_u64 << n) - 1) return 0;
    const auto initial_zeros = static_cast<unsigned>(std::countr_zero(mask));
    const auto subsequent_ones = static_cast<unsigned>(std::countr_one(mask >> initial_zeros));
    const unsigned total{initial_zeros + subsequent_ones};
    const std::uint64_t p{one_u64 << total};
    mask &= ~(p - 1);
    if (total == n) mask |= (one_u64 << (subsequent_ones + 1)) - 1;
    else {
        mask |= (one_u64 << (subsequent_ones - 1)) - 1;
        mask |= p;
    }
    return mask;
}

std::uint64_t find_cvc_by_bitmasking(const Simple_graph& g)
{
    std::uint64_t selection{1};
    while (selection != 0 && !selection_is_cvc(g, selection))
        selection = next_selection(static_cast<unsigned>(g.order()), selection);
    return selection;
}
