// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <istream>
#include <vector>

#include "simple_graph.h"

Simple_graph::Simple_graph(const int order) : _n{validate_order(order)}, _adj(order) {}

[[nodiscard]] int Simple_graph::order() const { return _n; }

[[nodiscard]] int Simple_graph::size() const { return _m; }

[[nodiscard]] std::vector<std::vector<int>> Simple_graph::adj_data() const { return _adj; }

[[nodiscard]] std::vector<int> Simple_graph::neighbors(const int v) const { return _adj.at(v); }

[[nodiscard]] int Simple_graph::degree(const int v) const { return static_cast<int>(_adj.at(v).size()); }

void Simple_graph::add_edge(const int u, const int v)
{
    if (u == v) throw std::invalid_argument{"edges must not be self-loops"};
    _adj.at(u).push_back(v);
    _adj.at(v).push_back(u);
    ++_m;
}

bool Simple_graph::remove_edge(const int u, const int v)
{
    auto& u_neighs = _adj.at(u);
    auto& v_neighs = _adj.at(v);

    const auto v_itr = std::ranges::find(u_neighs, v);
    if (v_itr == u_neighs.end()) return false;

    u_neighs.erase(v_itr);
    v_neighs.erase(std::ranges::find(v_neighs, u));
    --_m;
    return true;
}

int Simple_graph::validate_order(const int order)
{
    if (order <= 0) throw std::invalid_argument{"order must be positive"};
    return order;
}
