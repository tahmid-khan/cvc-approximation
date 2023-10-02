#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <iostream>
#include <numeric>
#include <vector>

template<typename T> T get_input(std::istream& fin);
std::vector<int> get_core_numbers(const std::vector<std::vector<int>>& adj);
std::vector<int> get_degeneracy_order(const std::vector<std::vector<int>>& adj);

int main(int argc, char const* argv[])
{
    if (argc != 2 || argv[1] == nullptr) {
        std::cerr << "Usage: " << argv[0] << " <input file path>\n";
        return EXIT_FAILURE;
    }

    /// input stream for the file containing the graph data
    std::ifstream fin {argv[1]};
    if (!fin) {
        std::cerr << "Error: cannot open file \"" << argv[1] << "\"\n";
        return EXIT_FAILURE;
    }

    // throw exceptions when input format is incorrect
    fin.exceptions(std::ios_base::failbit);

    auto n = get_input<int>(fin); ///< number of vertices
    auto m = get_input<int>(fin); ///< number of edges

    std::vector<std::vector<int>> adj(n); ///< the adjency list
    for (int i {0}; i < m; ++i) {
        auto u = get_input<int>(fin);
        auto v = get_input<int>(fin);
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    std::vector<int> degeneracy_order {get_degeneracy_order(adj)};
    std::cout << "degeneracy order:";
    for (auto v : degeneracy_order)
        std::cout << ' ' << v;

    std::vector<int> core_numbers {get_core_numbers(adj)};
    std::cout << "\n\ncore numbers:\n";
    for (int v {0}; v < n; ++v)
        std::cout << "vertex " << v << ": " << core_numbers[v] << '\n';

    return 0;
}

template<typename T> T get_input(std::istream& fin)
{
    T buf;
    fin >> buf;
    return buf;
}

std::vector<int> get_core_numbers(const std::vector<std::vector<int>>& adj)
{
    auto n = int(adj.size());

    /// degrees (# of neighbors) of the vertices; becomes the respective core numbers
    /// by the end of the algorith.
    std::vector<int> degrees(n);
    int max_degree {0};
    std::ranges::transform(adj, degrees.begin(), [&](const std::vector<int>& neighbors) {
        auto degree = int(neighbors.size());
        max_degree = std::max(max_degree, degree);
        return degree;
    });

    /// `bucket_offsets[d]` == offset (starting index) in the `vertices` array for the
    /// "bucket" of d-degree vertices
    std::vector<int> bucket_offsets(max_degree + 1, 0);
    for (auto d : degrees)
        ++bucket_offsets[d];
    std::partial_sum(bucket_offsets.begin(), bucket_offsets.end(), bucket_offsets.begin());
    std::shift_right(bucket_offsets.begin(), bucket_offsets.end(), 1);
    bucket_offsets.front() = 0;

    std::vector<int> vertices(n); ///< vertices in ascending order of degree
    std::vector<int> indices(n); ///< `indices[v]` == position of v in `vertices`
    for (int v {0}; v < n; ++v) {
        auto deg = degrees[v];
        auto idx = bucket_offsets[deg];
        vertices[idx] = v;
        indices[v] = idx;
        ++bucket_offsets[deg];
    }
    std::shift_right(bucket_offsets.begin(), bucket_offsets.end(), 1);
    bucket_offsets.front() = 0;

    for (int i {0}; i < n; ++i) {
        auto v = vertices[i];
        for (auto u : adj[v]) {
            if (auto& u_deg = degrees[u]; degrees[v] < u_deg) {
                auto& u_idx = indices[u];
                auto& buck_start = bucket_offsets[u_deg];
                auto& first_in_buck = vertices[buck_start];

                std::swap(first_in_buck, vertices[u_idx]);
                indices[first_in_buck] = u_idx;
                u_idx = buck_start;
                ++buck_start;
                --u_deg;
            }
        }
    }

    return degrees;
}

std::vector<int> get_degeneracy_order(const std::vector<std::vector<int>>& adj)
{
    auto n = int(adj.size());

    /// degrees (# of neighbors) of the vertices
    std::vector<int> degrees(n);
    int max_degree {0};
    std::ranges::transform(adj, degrees.begin(), [&](const std::vector<int>& neighbors) {
        auto degree = int(neighbors.size());
        max_degree = std::max(max_degree, degree);
        return degree;
    });

    /// `bucket_offsets[d]` == offset (starting index) in the `vertices` array for the
    /// "bucket" of d-degree vertices
    std::vector<int> bucket_offsets(max_degree + 1, 0);
    for (auto d : degrees)
        ++bucket_offsets[d];
    std::partial_sum(bucket_offsets.begin(), bucket_offsets.end(), bucket_offsets.begin());
    std::shift_right(bucket_offsets.begin(), bucket_offsets.end(), 1);
    bucket_offsets.front() = 0;

    std::vector<int> vertices(n); ///< vertices in ascending order of degree
    std::vector<int> indices(n); ///< `indices[v]` = position of v in `vertices`
    for (int v {0}; v < n; ++v) {
        auto deg = degrees[v];
        auto idx = bucket_offsets[deg];
        vertices[idx] = v;
        indices[v] = idx;
        ++bucket_offsets[deg];
    }
    std::shift_right(bucket_offsets.begin(), bucket_offsets.end(), 1);
    bucket_offsets.front() = 0;

    std::vector<int> res;
    res.reserve(n);
    for (int i {0}; i < n; ++i) {
        auto v = vertices[i];
        res.push_back(v);

        for (auto u : adj[v]) {
            if (auto& u_deg = degrees[u]; degrees[v] < u_deg) {
                auto& u_idx = indices[u];
                auto& buck_start = bucket_offsets[u_deg];
                auto& first_in_buck = vertices[buck_start];

                std::swap(first_in_buck, vertices[u_idx]);
                indices[first_in_buck] = u_idx;
                u_idx = buck_start;
                ++buck_start;
                --u_deg;
            }
        }
    }

    return res;
}
