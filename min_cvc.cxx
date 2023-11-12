#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <ios>
#include <iostream>
#include <set>
#include <utility>
#include <vector>

#include "adjacency_list_graph.h"

using std::any_of;
using std::cerr;
using std::cin;
using std::cout;
using std::fill;
using std::fill_n;
using std::find;
using std::function;
using std::ios_base;
using std::istream;
using std::max;
using std::min;
using std::next;
using std::pair;
using std::prev;
using std::set;
using std::strcmp;
using std::vector;
namespace ranges = std::ranges;

// clang-format off
// ReSharper disable CppFunctionalStyleCast, CppTemplateArgumentsCanBeDeduced, CppParameterMayBeConst, CppLocalVariableMayBeConst
// clang-format on

template<typename T> T scan(istream& is = cin);
bool selection_is_cvc(const Adjacency_list_graph& g, const vector<bool>& is_selected);
bool update_selection(vector<bool>& is_selected);
void print_true_indices(vector<bool> bools);

int main(const int argc, const char* argv[])
{
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    cin.exceptions(ios_base::failbit);

    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if (argc > 2 || (argc == 2 && strcmp(argv[1], "-n") != 0)) {
        cerr << "Usage: " << argv[0] << " [-n]\n";
        return EXIT_FAILURE;
    }
    // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    /// output mode: false => vertices, true => cardinality
    const bool out_mode{argc == 2};

    Adjacency_list_graph g(scan<int>());
    g.read_edges(scan<int>());

    vector<bool> is_selected(g.order(), false);
    is_selected[0] = true;

    do { // NOLINT(cppcoreguidelines-avoid-do-while)
        if (selection_is_cvc(g, is_selected)) {
            if (out_mode) cout << ranges::count(is_selected, true);
            else print_true_indices(is_selected);
            cout << '\n';
            break;
        }
    } while (update_selection(is_selected));

    return 0;
}

template<typename T> T scan(istream& is)
{
    T buf;
    is >> buf;
    return buf;
}

bool selection_is_cvc(const Adjacency_list_graph& g, const vector<bool>& is_selected)
{
    struct Edge : pair<int, int> {
        Edge(int u, int v)
        {
            first = min(u, v);
            second = max(u, v);
        }
    };
    set<Edge> covered;
    vector<bool> is_visited(g.order(), false);

    const function<void(int)> visit_connected = [&](int v) {
        is_visited[v] = true;
        for (auto u : g.neighbors(v))
            if (is_selected[u] && !is_visited[u]) visit_connected(u);
    };
    visit_connected(int(ranges::find(is_selected, true) - is_selected.begin()));

    for (int v{0}; v < g.order(); ++v) {
        if (is_selected[v]) {
            if (!is_visited[v]) return false;
            for (auto u : g.neighbors(v))
                covered.emplace(v, u);
        }
    }

    return int(covered.size()) == g.size();
}

bool update_selection(vector<bool>& is_selected)
{
    auto first_selection = ranges::find(is_selected, true);
    auto to_select = find(first_selection, is_selected.end(), false);

    if (to_select == is_selected.end()) {
        if (first_selection == is_selected.begin()) return false;

        fill(first_selection, to_select, false);
        fill_n(is_selected.begin(), to_select - first_selection + 1, true);
        return true;
    }

    *to_select = true;
    if (any_of(next(to_select), is_selected.end(), [](bool yes) { return yes; })) {
        *prev(to_select) = false;
        return true;
    }

    fill(first_selection, to_select, false);
    fill_n(is_selected.begin(), to_select - first_selection - 1, true);
    return true;
}

void print_true_indices(vector<bool> bools)
{
    bool sep{false};
    for (int v{0}; v < int(bools.size()); ++v) {
        if (bools[v]) {
            if (sep) cout << ' ';
            cout << v;
            sep = true;
        }
    }
}
