// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#ifndef ORDERING_H_INCLUDED
#define ORDERING_H_INCLUDED

#include <vector>

#include "simple_graph.h"

std::vector<int> descending_degree_ordering(const Simple_graph& g);
std::vector<int> smallest_last_ordering(const Simple_graph& g);
std::vector<int> smallest_log_last_ordering(const Simple_graph& g, const int max_rounds);
std::vector<int> saturation_degree_ordering(const Simple_graph& g);

#endif // ORDERING_H_INCLUDED
