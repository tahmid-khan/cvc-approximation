// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <algorithm>
#include <utility>
#include <vector>

#include "simple_graph.h"

// ReSharper disable CppTemplateArgumentsCanBeDeduced

std::pair<Simple_graph, int> dfs_traverse(const Simple_graph& g, const int root, const std::vector<int>& priority)
{
    auto adj = g.adj_data();
    const auto cmp = [&priority](const int u1, const int u2) {
        return priority[u1] > priority[u2];
    };
    for (auto& neighbors : adj)
        std::ranges::sort(neighbors, cmp);

    Simple_graph dfs_tree{g.order()};
    std::vector<bool> is_visited(g.order(), false);
    std::vector<std::pair<int, int>> sources_stack{{std::make_pair(root, 0)}};
    is_visited[root] = true;
    int leaf_adj_to_root{-1};

    while (!sources_stack.empty()) {
        auto& [source, neigh_offset] = sources_stack.back();
        const auto& neighbors = adj[source];

        const auto dest_itr =
            std::find_if_not(neighbors.cbegin() + neigh_offset,
                             neighbors.cend(),
                             [&is_visited](const int u) { return is_visited[u]; });
        if (dest_itr == neighbors.cend()) {
            if (leaf_adj_to_root == -1 && dfs_tree.degree(source) == 1 &&
                std::ranges::find(neighbors, root) != neighbors.cend()) {
                leaf_adj_to_root = source;
            }
            sources_stack.pop_back();
            continue;
        }
        const int dest{*dest_itr};

        dfs_tree.add_edge(source, dest);
        is_visited[dest] = true;
        neigh_offset = static_cast<int>(dest_itr - neighbors.cbegin()) + 1;
        sources_stack.emplace_back(dest, 0);
    }

    return std::make_pair(dfs_tree, leaf_adj_to_root);
}

std::pair<int, int> closest_branch(const Simple_graph& tree, const int leaf)
{
    std::pair<int, int> child_parent{-1, leaf};
    const auto move_up = [&]() -> bool {
        const auto& [c, p] = child_parent;
        const auto& neighs = tree.neighbors(p);
        const auto itr = std::ranges::find_if(neighs, [&](const int u) -> bool { return u != c; });

        if (itr == neighs.cend()) {
            child_parent = {p, -1};
            return false;
        }

        child_parent = {p, *itr};
        return true;
    };

    while (move_up())
        if (tree.degree(child_parent.second) >= 3) break;
    return child_parent;
}

Simple_graph ilst(const Simple_graph& g, const std::vector<int>& priority)
{
    const auto root = static_cast<int>(std::ranges::max_element(priority) - priority.cbegin());

    auto [tree, l] = dfs_traverse(g, root, priority);
    if (l == -1) return tree;

    if (const auto& [bl_child, bl] = closest_branch(tree, l); bl_child != -1 && bl != -1) {
        tree.remove_edge(bl_child, bl);
        tree.add_edge(l, root);
    }
    return tree;
}
