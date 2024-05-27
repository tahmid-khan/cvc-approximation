#include "backtracking.h"

#include <algorithm>
#include <iterator>
#include <set>
#include <utility>
#include <vector>

std::pair<int, int> make_edge(int u, int v)
{
    return std::make_pair(std::min(u, v), std::max(u, v));
}

int recur(const Simple_graph& g, // NOLINT(misc-no-recursion)
          std::set<std::pair<int, int>>& covered,
          std::set<int>& candidates,
          std::set<int>& taken)
{
    if (std::ssize(covered) == g.size()) return static_cast<int>(taken.size());

    std::set<int> min_cvc;
    int min_cardinality{g.order()};
    while (!candidates.empty()) {
        const int v{*candidates.begin()};
        candidates.erase(candidates.begin());

        std::vector<int> newly_covered;
        for (const auto u : g.neighbors(v)) {
            candidates.insert(u);
            if (const auto& [_, is_new] = covered.emplace(make_edge(u, v)); is_new)
                newly_covered.push_back(u);
        }

        if (const int c{recur(g, covered, candidates, taken)}; c < min_cardinality) {
            min_cardinality = c;
            min_cvc = taken;
        }
        taken.insert(v);
        if (const int c{recur(g, covered, candidates, taken)}; c < min_cardinality) {
            min_cardinality = c;
            min_cvc = taken;
        }
        taken.erase(v);

        for (const auto u : newly_covered)
            covered.erase(make_edge(u, v));
    }

    taken = min_cvc;
    return static_cast<int>(taken.size());
}

std::set<int> find_cvc_by_backtracking(const Simple_graph& g)
{
    std::set<int> best;
    int best_cardinality{g.order()};
    std::set<int> taken;
    std::set<int> candidates;
    std::set<std::pair<int, int>> covered;
    for (int v{0}; v < g.order(); ++v) {
        std::vector<int> newly_covered;
        for (const auto u : g.neighbors(v)) {
            candidates.insert(u);
            if (covered.emplace(make_edge(u, v)).second) newly_covered.push_back(u);
        }

        if (const int c{recur(g, covered, candidates, taken)}; c < best_cardinality) {
            best = taken;
            best_cardinality = c;
        }
        taken.insert(v);
        if (const int c{recur(g, covered, candidates, taken)}; c < best_cardinality) {
            best = taken;
            best_cardinality = c;
        }
        taken.erase(v);

        for (const auto u : newly_covered)
            covered.erase(make_edge(u, v));
    }
    return best;
}
