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
    [[nodiscard]] std::vector<std::vector<int>> adj() const { return _adj; }

    /// Returns the neighbors of vertex `v`.
    [[nodiscard]] std::vector<int> neighbors(int v) const { return _adj.at(v); }

    /// Reads the edges of the graph from the given input stream.
    /// Each edge should be given as a pair of integers separated by one or
    /// more whitespaces, and the edges themselves should be separated likewise.
    /// @param max_edges non-negative integer giving the number of edges to read
    /// @param is the input stream from to read the edges from;
    ///           default: `std::cin`
    /// @throws std::invalid_argument if `max_edges` is negative
    void read_edges(int max_edges, std::istream& is = std::cin)
    {
        if (max_edges < 0) throw std::invalid_argument("max_edges must not be negative");
        while (max_edges-- != 0) {
            int u, v; // NOLINT
            is >> u >> v;
            add_edge(u, v);
        }
    }

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
