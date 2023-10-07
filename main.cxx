#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <iostream>
#include <numeric>
#include <stack>
#include <utility>
#include <vector>

using std::cerr;
using std::cout;
using std::ifstream;
using std::ios_base;
using std::istream;
using std::make_pair;
using std::max;
using std::min_element;
using std::pair;
using std::partial_sum;
using std::shift_right;
using std::stack;
using std::swap;
using std::vector;
namespace ranges = std::ranges;
using Vertex_list = vector<int>;
using Adjacency_list = vector<Vertex_list>;

template<typename T> T get_input(istream& is);
vector<int> get_core_numbers(const Adjacency_list& adj);
Vertex_list get_degeneracy_order(const Adjacency_list& adj);
Adjacency_list dfs_degeneracy_order(int root, Adjacency_list adj, const vector<int>& core_numbers, bool reverse = false);

int main(int argc, char const* argv[])
{
    if (argc != 2 || argv[1] == nullptr) {
        cerr << "Usage: " << argv[0] << " <input file path>\n";
        return EXIT_FAILURE;
    }

    /// input stream for the file containing the graph data
    ifstream fin {argv[1]};
    if (!fin) {
        cerr << "Error: cannot open file \"" << argv[1] << "\"\n";
        return EXIT_FAILURE;
    }

    // throw exceptions when input is of incorrect format
    fin.exceptions(ios_base::failbit);

    auto n = get_input<int>(fin); ///< number of vertices
    auto m = get_input<int>(fin); ///< number of edges

    Adjacency_list adj(n); ///< the adjacency list
    for (int i {0}; i < m; ++i) {
        auto u = get_input<int>(fin);
        auto v = get_input<int>(fin);
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    const Vertex_list degeneracy_order {get_degeneracy_order(adj)};
    cout << "degeneracy order:";
    for (auto v : degeneracy_order)
        cout << ' ' << v;

    const vector<int> core_numbers {get_core_numbers(adj)};
    cout << "\n\ncore numbers:\n";
    for (int v {0}; v < n; ++v)
        cout << "vertex " << v << ": " << core_numbers[v] << '\n';

    auto min_degree_v = int(min_element(core_numbers.cbegin(), core_numbers.cend()) - core_numbers.cbegin());
    Adjacency_list dfs_tree {dfs_degeneracy_order(min_degree_v, adj, core_numbers)};

    cout << "\nDFS tree (adjacency list):\n";
    for (int v {0}; v < n; ++v) {
        cout << v << ':';
        for (auto u : dfs_tree[v])
            cout << ' ' << u;
        cout << '\n';
    }

    return 0;
}

Adjacency_list dfs_degeneracy_order(int root, Adjacency_list adj, const vector<int>& core_numbers, bool reverse)
{
    // sort neighbors in degeneracy order (or reverse degeneracy order)
    for (auto& neighbors : adj) {
        ranges::sort(neighbors, [&](int u1, int u2) {
            return reverse
                ? (core_numbers[u1] > core_numbers[u2])
                : (core_numbers[u1] < core_numbers[u2]);
        });
    }

    Adjacency_list tree_adj(adj.size());
    vector<bool> visited(adj.size(), false);
    stack<pair<int, int>> edge_sources {{make_pair(root, 0)}};

    while (!edge_sources.empty()) {
        auto& [source, neigh_offset] = edge_sources.top();
        visited[source] = true;

        const auto& neighbors = adj[source];
        auto dest_itr = find_if_not(neighbors.cbegin() + neigh_offset,
                                    neighbors.cend(),
                                    [&visited](int neigh) { return visited[neigh]; });
        if (dest_itr == neighbors.end()) {
            edge_sources.pop();
            continue;
        }

        tree_adj[source].push_back(*dest_itr);
        edge_sources.emplace(*dest_itr, 0);
        neigh_offset = int(dest_itr - neighbors.begin()) + 1;
    }

    return tree_adj;
}

template<typename T> T get_input(istream& is)
{
    T buf;
    is >> buf;
    return buf;
}

vector<int> get_core_numbers(const Adjacency_list& adj)
{
    auto n = int(adj.size());

    /// degrees (# of neighbors) of the vertices; this becomes the respective core numbers
    /// by the end of the algorithm
    vector<int> degrees(n);
    int max_degree {0};
    ranges::transform(adj, degrees.begin(), [&max_degree](const Vertex_list& neighbors) {
        auto degree = int(neighbors.size());
        max_degree = max(max_degree, degree);
        return degree;
    });

    vector<int> degree_freqs(max_degree + 1, 0); ///< frequencies of degrees
    for (auto d : degrees)
        ++degree_freqs[d];

    /// `degree_offsets[d]` == starting index (offset) of the subarray of d-degree vertices
    /// in the `vertices` array
    auto& degree_offsets = degree_freqs;
    partial_sum(degree_offsets.begin(), degree_offsets.end(), degree_offsets.begin());
    shift_right(degree_offsets.begin(), degree_offsets.end(), 1);
    degree_offsets.front() = 0;

    Vertex_list vertices(n); ///< vertices in ascending order of degree
    vector<int> indices(n); ///< `indices[v]` == position of v in `vertices`
    for (int v {0}; v < n; ++v) {
        auto d = degrees[v];
        auto i = degree_offsets[d];
        vertices[i] = v;
        indices[v] = i;
        ++degree_offsets[d];
    }
    shift_right(degree_offsets.begin(), degree_offsets.end(), 1);
    degree_offsets.front() = 0;

    for (int i {0}; i < n; ++i) {
        auto v = vertices[i];
        for (auto u : adj[v]) {
            if (auto& du = degrees[u]; degrees[v] < du) {
                auto& firsti = degree_offsets[du]; ///< index of the first du-degree vertex
                auto& firstv = vertices[firsti]; ///< first du-degree vertex
                auto& iu = indices[u];

                // reduce u's degree by 1, since the neighbor v is removed
                --du;

                // swap u with the first du-degree vertex
                swap(vertices[iu], firstv);

                // update their indices
                indices[firstv] = iu;
                iu = firsti;

                // the subarray with vertices having the same degree as the previous degree of u
                // now starts at one position to the right
                ++firsti;
            }
        }
    }

    return degrees;
}

Vertex_list get_degeneracy_order(const Adjacency_list& adj)
{
    auto n = int(adj.size());

    /// degrees (# of neighbors) of the vertices; this becomes the respective core numbers
    /// by the end of the algorithm
    vector<int> degrees(n);
    int max_degree {0};
    ranges::transform(adj, degrees.begin(), [&max_degree](const Vertex_list& neighbors) {
        auto degree = int(neighbors.size());
        max_degree = max(max_degree, degree);
        return degree;
    });

    vector<int> degree_freqs(max_degree + 1, 0); ///< frequencies of degrees
    for (auto d : degrees)
        ++degree_freqs[d];

    /// `degree_offsets[d]` == starting index (offset) of the subarray of d-degree vertices
    /// in the `vertices` array
    auto& degree_offsets = degree_freqs;
    partial_sum(degree_offsets.begin(), degree_offsets.end(), degree_offsets.begin());
    shift_right(degree_offsets.begin(), degree_offsets.end(), 1);
    degree_offsets.front() = 0;

    Vertex_list vertices(n); ///< vertices in ascending order of degree
    vector<int> indices(n); ///< `indices[v]` == position of v in `vertices`
    for (int v {0}; v < n; ++v) {
        auto d = degrees[v];
        auto i = degree_offsets[d];
        vertices[i] = v;
        indices[v] = i;
        ++degree_offsets[d];
    }
    shift_right(degree_offsets.begin(), degree_offsets.end(), 1);
    degree_offsets.front() = 0;

    Vertex_list res;
    res.reserve(n);
    for (int i {0}; i < n; ++i) {
        auto v = vertices[i];
        res.push_back(v);

        for (auto u : adj[v]) {
            if (auto& du = degrees[u]; degrees[v] < du) {
                auto& firsti = degree_offsets[du]; ///< index of the first du-degree vertex
                auto& firstv = vertices[firsti]; ///< first du-degree vertex
                auto& iu = indices[u];

                // reduce u's degree by 1, since the neighbor v is removed
                --du;

                // swap u with the first du-degree vertex in the `vertices` array
                swap(vertices[iu], firstv);

                // update their indices
                indices[firstv] = iu;
                iu = firsti;

                // the subarray with vertices having the same degree as the previous degree of u
                // now starts at one position to the right
                ++firsti;
            }
        }
    }

    return res;
}
