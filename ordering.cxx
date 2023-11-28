// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "ordering.h"

#include <algorithm>
#include <iterator>
#include <numeric>
#include <set>
#include <tuple>
#include <utility>
#include <vector>

#include "simple_graph.h"

std::vector<int> descending_degree_ordering(const Simple_graph& g)
{
    std::vector<int> res(g.order());
    std::iota(res.begin(), res.end(), 0);
    std::ranges::sort(res, [&](int u, int v) -> bool {
        return std::make_pair(-g.degree(u), u) < std::make_pair(-g.degree(v), v);
    });
    return res;
}

std::vector<int> smallest_last_ordering(const Simple_graph& g)
{
    std::vector<int> degree(g.order());
    const auto cmp = [&](const int u, const int v) -> bool {
        return std::make_pair(degree[u], u) < std::make_pair(degree[v], v);
    };
    std::set<int, decltype(cmp)> todo{cmp};
    for (int v{0}; v < g.order(); ++v) {
        degree[v] = g.degree(v);
        todo.insert(v);
    }

    std::vector<int> res(g.order(), -1);
    int index{g.order() - 1};
    while (!todo.empty()) {
        const int v{*todo.begin()};
        todo.erase(todo.begin());
        res[index--] = v;

        for (const auto u : g.neighbors(v)) {
            const auto itr = todo.find(u);
            if (itr == todo.cend()) continue;

            todo.erase(itr);
            --degree[u];
            todo.insert(u);
        }
    }

    return res;
}

std::vector<int> smallest_log_last_ordering(const Simple_graph& g, const int max_rounds)
{
    std::vector<std::set<int>> adj(g.order());
    const auto cmp = [&](int u, int v) -> bool {
        return std::make_pair(adj[u].size(), u) < std::make_pair(adj[v].size(), v);
    };
    std::set<int, decltype(cmp)> todo{cmp};
    for (int v{0}; auto&& neighs : g.adj_data()) {
        adj[v] = std::set<int>(std::make_move_iterator(neighs.begin()),
                               std::make_move_iterator(neighs.end()));
        todo.insert(v);
        ++v;
    }

    std::vector<int> res(g.order(), -1);
    int index{g.order() - 1};
    for (unsigned d{0}; index >= 0 && !todo.empty(); ++d) {
        for (int round{0}; round < max_rounds && !todo.empty(); ++round) {
            const int v{*todo.begin()};
            if (std::bit_ceil(adj[v].size()) > d) break;

            todo.erase(todo.begin());
            res[index--] = v;

            for (const auto u : adj[v]) {
                todo.erase(u);
                adj[u].erase(v);
                todo.insert(u);
            }
        }
    }

    return res;
}

std::vector<int> saturation_degree_ordering(const Simple_graph& g)
{
    /// The sets of colors used by neighbors
    /// (`adj_colors[v]` == v.adjColors in the paper's notation)
    std::vector<std::set<int>> adj_colors(g.order());

    /// The sets of uncolored neighbors
    /// (`adj_uncolored[v]` == v.adjUncolored in the paper's notation)
    std::vector<std::set<int>> adj_uncolored(g.order());

    const auto smallest_available_color = [&](const int v) -> int {
        int prev = -1;
        for (const auto curr : adj_colors[v]) {
            if (prev + 1 != curr) break;
            prev = curr;
        }
        return prev + 1;
    };

    /// Comparator for sorting vertices
    ///   1. first in decreasing order of saturation degrees
    ///      (|v.adjColors| in the paper's notation),
    ///   2. then in decreasing order of numbers of uncolored neighbors (|v.adjUncolored|),
    ///   3. then in decreasing order of degrees (|v.adj|),
    ///   4. and finally in incresing order of v
    const auto cmp = [&](int u, int v) -> bool {
        return std::make_tuple(adj_colors[u].size(), adj_uncolored[u].size(), g.degree(u), v) >
               std::make_tuple(adj_colors[v].size(), adj_uncolored[v].size(), g.degree(v), u);
    };
    std::set<int, decltype(cmp)> uncolored{cmp};

    // Initialize the data structures. [Lines 39-40 of the paper's pseudocode]
    for (int v{0}; auto&& neighs : g.adj_data()) {
        adj_uncolored[v] = std::set<int>(std::make_move_iterator(neighs.begin()),
                                         std::make_move_iterator(neighs.end()));
        uncolored.insert(v);
        ++v;
    }

    std::vector<int> res(g.order(), -1);
    int index{g.order() - 1};

    // The main loop [Lines 42-52 of the paper's pseudocode]
    while (!uncolored.empty()) {
        // [Line 43]
        const int v{*uncolored.begin()};
        uncolored.erase(uncolored.begin());
        res[index--] = v;

        // [Line 44]
        const int c{smallest_available_color(v)};

        // [Lines 45-50: the for loop]
        for (const auto u : adj_uncolored[v]) {
            uncolored.erase(u);        // [Line 46]
            adj_colors[u].insert(c);   // [Line 47]
            adj_uncolored[u].erase(v); // [Line 48]
            uncolored.insert(u);       // [Line 49]
        }
    }

    return res;
}
