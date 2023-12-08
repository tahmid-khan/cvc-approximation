// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <ios>
#include <iostream>
#include <iterator>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "ilst.h"
#include "ordering.h"
#include "simple_graph.h"

// ReSharper disable CppTemplateArgumentsCanBeDeduced

constexpr int r_limit{32};
enum class Ordering_strategy : std::uint8_t { dd, sl, sll, sd };

template<typename T> constexpr T scan(std::istream& is = std::cin)
{
    T buf;
    is >> buf;
    return buf;
}

void show_usage(const char* const exe)
{
    std::cerr << "Usage: " << exe
              << " <ordering heuristic: (dd|sl|sll|sd)> [<the r parameter if heuristic is sll>]:\n"
              << '\t' << exe << " dd\n"
              << '\t' << exe << " sl\n"
              << '\t' << exe << " sll <r>\n"
              << '\t' << exe << " sd\n";
}

bool is_connected(const Simple_graph& g)
{
    std::vector<bool> is_visited(g.order(), false);
    const std::function<void(const int)> dfs = [&](const int v) {
        is_visited[v] = true;
        for (const auto u : g.neighbors(v))
            if (!is_visited[u]) dfs(u);
    };
    dfs(0);

    // NOLINTNEXTLINE(*-simplify-boolean-expr)
    return std::ranges::all_of(is_visited, [](const bool status) { return status == true; });
}

std::vector<int> ordering_to_priorities(const std::vector<int>& ordering)
{
    std::vector<int> priority(ordering.size());
    const auto n = static_cast<int>(ordering.size());
    for (int i{0}; i < n; ++i)
        priority[ordering[i]] = n - i - 1;
    return priority;
}

std::vector<int> cvc_from_ilst(const Simple_graph& g, const Simple_graph& ilst_tree)
{
    std::vector<int> res;
    for (int v{0}; v < ilst_tree.order(); ++v)
        if (ilst_tree.degree(v) > 1) res.push_back(v);

    // One of the leaves should be included iff g is a ring.
    // And if g is a ring, ilst_tree has two leaves and they are not g-independent.
    if (const int nleaves{g.order() - static_cast<int>(res.size())}; nleaves == 2) {
        int l1{0};
        for (; l1 < ilst_tree.order(); ++l1)
            if (ilst_tree.degree(l1) == 1) break;

        int l2{l1 + 1};
        for (; l2 < ilst_tree.order(); ++l2)
            if (ilst_tree.degree(l2) == 1) break;

        // if l1 and l2 are not g-independent, add l1 to the result
        if (const auto& nn = g.neighbors(l1); std::ranges::find(nn, l2) != nn.cend())
            res.push_back(l1);
    }

    return res;
}

/// Checks if the `selection` of vertices forms a connected vertex cover of `g`.
bool selection_is_cvc(const Simple_graph& g, const std::vector<int>& selection)
{
    std::vector<bool> is_selected(g.order(), false);

    // check if all edges are covered
    std::set<std::pair<int, int>> covered;
    for (const auto v : selection) {
        is_selected[v] = true;
        for (const auto u : g.neighbors(v))
            covered.emplace(std::min(u, v), std::max(u, v));
    }
    if (std::ssize(covered) != g.size()) return false;

    std::vector<bool> is_visited(g.order(), false);

    // visit all vertices connected to root
    const std::function<void(const int)> visit_connected = [&](const int v) {
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

void print_integers(const std::vector<int>& ints)
{
    for (unsigned i{0}; i < ints.size(); ++i) {
        if (i != 0) std::cout << ' ';
        std::cout << ints[i];
    }
}

int main(const int argc, const char* const argv[])
{
    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    Ordering_strategy strategy; // NOLINT(*-init-variables)
    int r{-1};

    if (argc == 3) {
        if (std::strcmp(argv[1], "sll") != 0) {
            show_usage(argv[0]);
            return EXIT_FAILURE;
        }

        try {
            if (const unsigned long ul{std::stoul(argv[2])}; 1 <= ul && ul <= r_limit)
                r = static_cast<int>(ul);
        }
        catch (const std::logic_error&) {
            std::cerr << "r must be in the range [1, " << r_limit << "]\n";
            return EXIT_FAILURE;
        }
        if (r == -1) {
            std::cerr << "r must be in the range [1, " << r_limit << "]\n";
            return EXIT_FAILURE;
        }

        strategy = Ordering_strategy::sll;
    }
    else if (argc == 2) {
        if (std::strcmp(argv[1], "dd") == 0) strategy = Ordering_strategy::dd;
        else if (std::strcmp(argv[1], "sl") == 0) strategy = Ordering_strategy::sl;
        else if (std::strcmp(argv[1], "sd") == 0) strategy = Ordering_strategy::sd;
        else {
            show_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }
    else {
        show_usage(argv[0]);
        return EXIT_FAILURE;
    }
    // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    // throw exceptions when input is of incorrect format
    std::cin.exceptions(std::ios_base::failbit);

    Simple_graph g(scan<int>());
    if (g.order() <= 1) {
        std::cerr << "Graph must have at least 2 vertices\n";
        return EXIT_FAILURE;
    }

    for (auto m = scan<int>(); m-- != 0;)
        g.add_edge(scan<int>(), scan<int>());
    if (!is_connected(g)) {
        std::cerr << "Graph must be connected\n";
        return EXIT_FAILURE;
    }

    std::vector<int> ordering;
    switch (strategy) {
    case Ordering_strategy::dd:
        ordering = descending_degree_ordering(g);
        break;
    case Ordering_strategy::sl:
        ordering = smallest_last_ordering(g);
        break;
    case Ordering_strategy::sll:
        ordering = smallest_log_last_ordering(g, r);
        break;
    case Ordering_strategy::sd:
        ordering = saturation_degree_ordering(g);
        break;
    }

    // forward
    Simple_graph ilst_tree{ilst(g, ordering_to_priorities(ordering))};
    std::vector<int> cvc_approx{cvc_from_ilst(g, ilst_tree)};
    if (!selection_is_cvc(g, cvc_approx)) {
        std::cout << "-1\n";
        return EXIT_FAILURE;
    }
    print_integers(cvc_approx);
    std::cout << '\n';

    // reverse
    std::ranges::reverse(ordering);
    ilst_tree = ilst(g, ordering_to_priorities(ordering));
    cvc_approx = cvc_from_ilst(g, ilst_tree);
    if (!selection_is_cvc(g, cvc_approx)) return EXIT_FAILURE;
    print_integers(cvc_approx);
    std::cout << '\n';

    return 0;
}
