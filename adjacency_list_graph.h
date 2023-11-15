// -*- C++ -*-
#ifndef ADJACENCY_LIST_GRAPH_H_GUARD
#define ADJACENCY_LIST_GRAPH_H_GUARD

#include <iostream>
#include <istream>
#include <vector>

// ReSharper disable CppParameterMayBeConst

class Adjacency_list_graph {
public:
    /// @param order the number of vertices in the graph
    explicit Adjacency_list_graph(int order) : _n{validate_order(order)}, _adj(order) {}

    /// Returns the number of vertices in the graph.
    [[nodiscard]] int order() const { return _n; }

    /// Returns the number of edges in the graph.
    [[nodiscard]] int size() const { return _m; }

    /// Returns the adjacency list of the graph.
    [[nodiscard]] std::vector<std::vector<int>> adj_data() const { return _adj; }

    /// Returns the neighbors of vertex `v`.
    [[nodiscard]] std::vector<int> neighbors(int v) const { return _adj.at(v); }

    /// Returns the number of neighbors of vertex `v`.
    [[nodiscard]] int neighbor_count(int v) const { return int(_adj.at(v).size()); }

    /// Adds an edge between vertex `u` and vertex `v`.
    /// Amortized time complexity: O(1)
    void add_edge(int u, int v)
    {
        _adj.at(u).push_back(v);
        _adj.at(v).push_back(u);
        ++_m;
    }

private:
    int _n;                             ///< number of vertices
    int _m{0};                          ///< number of edges
    std::vector<std::vector<int>> _adj; ///< adjacency list

    static int validate_order(int order)
    {
        if (order <= 0) throw std::invalid_argument("order must be positive");
        return order;
    }
};

#endif // ADJACENCY_LIST_GRAPH_H_GUARD
