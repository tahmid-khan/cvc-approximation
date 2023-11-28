// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#ifndef GRAPH_H_INCLUDED
#define GRAPH_H_INCLUDED

#include <vector>

class Simple_graph {
public:
    /// @param order the number of vertices in the graph
    explicit Simple_graph(const int order);

    /// Returns the number of vertices in the graph.
    [[nodiscard]] int order() const;

    /// Returns the number of edges in the graph.
    [[nodiscard]] int size() const;

    /// Returns the adjacency list of the graph.
    [[nodiscard]] std::vector<std::vector<int>> adj_data() const;

    /// Returns the neighbors of vertex `v`.
    [[nodiscard]] std::vector<int> neighbors(const int v) const;

    /// Returns the number of neighbors of vertex `v`.
    [[nodiscard]] int degree(const int v) const;

    /// Adds an edge between vertex `u` and vertex `v`.
    /// Amortized time complexity: O(1)
    void add_edge(const int u, int v);

    /// Removes the edge (first if there are multiple) between vertex `u` and vertex `v`.
    /// Returns false iff `u` and `v` are not neighbors.
    /// Time complexity: O(degree(u) + degree(v))
    bool remove_edge(const int u, int v);

private:
    int _n;                             ///< number of vertices
    int _m{0};                          ///< number of edges
    std::vector<std::vector<int>> _adj; ///< adjacency list

    static int validate_order(const int order);
};

#endif // GRAPH_H_INCLUDED
