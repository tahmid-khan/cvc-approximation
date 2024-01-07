// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#ifndef ILST_H_INCLUDED
#define ILST_H_INCLUDED

#include <vector>

#include "simple_graph.h"

Simple_graph ilst(const Simple_graph& g, const std::vector<int>& priority);

#endif // ILST_H_INCLUDED
