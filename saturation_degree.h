// -*- C++ -*-
#ifndef SD_H_GUARD
#define SD_H_GUARD

#include <iterator>
#include <set>
#include <tuple>
#include <vector>

#include "adjacency_list_graph.h"

// ReSharper disable CppTemplateArgumentsCanBeDeduced

std::vector<int> ordering_by_sd(const Adjacency_list_graph& g)
{
    const auto& adj = g.adj_data();

    /// The sets of colors used by neighbors
    /// (`adj_colors[v]` == v.adjColors in the paper's notation)
    std::vector<std::set<int>> adj_colors(g.order());

    /// The sets of uncolored neighbors
    /// (`adj_uncolored[v]` == v.adjUncolored in the paper's notation)
    std::vector<std::set<int>> adj_uncolored(g.order());

    const auto smallest_available_color = [&](int v) -> int {
        int prev = -1;
        for (const auto curr : adj_colors[v]) {
            if (prev + 1 != curr) break;
            prev = curr;
        }
        return prev + 1;
    };

    /// Comparator for sorting vertices
    ///   1. first by decreasing order of saturation degree
    ///      (|v.adjColors| in the paper's notation),
    ///   2. then by decreasing order of number of uncolored neighbors (|v.adjUncolored|),
    ///   3. then by decreasing order of degree (|v.adj|),
    ///   4. and finally by incresing order of v
    const auto cmp = [&](int u, int v) -> bool {
        return
            std::tuple(adj_colors[u].size(), adj_uncolored[u].size(), adj[u].size(), v) >
            std::tuple(adj_colors[v].size(), adj_uncolored[v].size(), adj[v].size(), u);
    };

    /// The vertices, sorted
    ///   1. first by decreasing order of saturation degree (|v.adjColors|),
    ///   2. then by decreasing order of number of uncolored neighbors (|v.adjUncolored|),
    ///   3. then by decreasing order of degree (|v.adj|),
    ///   4. and finally by incresing order of v
    std::set<int, decltype(cmp)> uncolored {cmp};

    // Initialize the data structures. [Lines 39-40 of the paper's pseudocode]
    for (int v {0}; v < g.order(); ++v) {
        adj_uncolored[v] = std::set<int>(std::make_move_iterator(adj[v].begin()),
                                         std::make_move_iterator(adj[v].end()));
        uncolored.insert(v);
    }

    std::vector<int> order;
    order.reserve(g.order());

    // The main loop [Lines 42-52 of the paper's pseudocode]
    while (!uncolored.empty()) {
        // [Line 43: PopOrDelKey]
        const int v {*uncolored.begin()};
        uncolored.erase(uncolored.begin());

        order.push_back(v);

        // [Line 44]
        const int c {smallest_available_color(v)};

        // [Lines 45-50: for loop]
        for (auto u : adj_uncolored[v]) {
            auto& u_adj_colors = adj_colors[u];
            auto& u_adj_uncolored = adj_uncolored[u];

            uncolored.erase(u);       // [Line 46: RemoveOrDelKey]
            u_adj_colors.insert(c);   // [Line 47]
            u_adj_uncolored.erase(v); // [Line 48]
            uncolored.insert(u);      // [Line 49: PushOrAddKey]
        }
    }

    return order;
}

#endif // SD_H_GUARD
