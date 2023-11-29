## Building

A standards-compliant compiler toolchain that supports C++20 is required. The build instructions in the [Without Using CMake](#without-using-cmake) section assume that the compiler is G++.

### Using CMake

```bash
mkdir build # the directory to output the build files to
cmake -S . -B build # configure and generate the build files
cmake --build build # build the project using the generated build files
```

### Without Using CMake

```bash
mkdir build # the directory to output the built libraries and executables files to
g++ -std=c++20 -O2 -c simple_graph.cxx -o build/simple_graph.o # compile the graph class
g++ -std=c++20 -O2 -c ordering.cxx -o build/ordering.o # compile the library for the ordering heuristics
g++ -std=c++20 -O2 -c ilst.cxx -o build/ilst.o # compile the library for the ILST algorithm
g++ -std=c++20 -O2 min_cvc.cxx simple_graph.o -o build/min_cvc # compile the minimum connected vertex cover solver
g++ -std=c++20 -O2 ilst_cvc.cxx simple_graph.o ordering.o ilst.o -o build/ilst_cvc # compile the ILST-based CVC approximator
```

## Usage

A graph is taken from the standard input in the following format:

```
𝑛
𝑚
𝑢₁ 𝑣₁
𝑢₂ 𝑣₂
⋮
𝑢ₘ 𝑣ₘ
```

Here `𝑛` is the number of vertices, `𝑚` is the number of edges, and `𝑢ᵢ 𝑣ᵢ` are the endpoints of the `𝑖`th edge.
The program uses 0-based numbering for the vertices, so each `𝑢ᵢ` and each `𝑣ᵢ` must be in the range [0, 𝑛).

### Minimum Connected Vertex Cover

```bash
./build/min_cvc [-c]
```

By default the output will be a space separated list of the vertices that form the minimum connected vertex cover. Use the optional `-c` flag to output the cardinality (element-count) of the minimum connected vertex cover set instead.

To use the provided data files, redirect the standard input from the graph file like in the following example (shown using the `order_65-128/polbooks.txt` graph):

```bash
./build/min_cvc <data/prepared/order_65-128/polbooks.txt
```

### ILST-based Minimum Connected Vertex Cover Approximator

The ILST-CVC program requires a command-line flag that `〈heuristic〉` specifies the vertex-ordering heuristic to be used during the DFS traversal in the ILST algorithm. The available heuristics are:

- `dd` - decreasing-degree;
- `sl` - smallest-degree-last (reverse degeneracy ordering);
- `sll` - smallest-log-degree-last (from [Hasenplaugh et al.][1]); takes an additional argument `𝑟` for the maximum number of times the SLL algorithm may remove vertices of at most a particular degree;
- `sd` - saturation-degree (high to low).

[1]: http://supertech.csail.mit.edu/papers/HasenplaughKaSc14.pdf

```bash
./build/ilst_cvc 〈heuristic〉 [𝑟]
```

The output will be two lines, each a space separated list of the vertices that form the approximate minimum connected vertex cover. The first line is for using the ordering heuristic as is (in forward direction); the second line is for using the ordering heuristic in reverse direction.

#### Example 1

```bash
./build/ilst_cvc dd
```

Then in the standard input:
```
5
3
1 2
2 3
2 4
```

#### Example 2

```bash
./build/ilst_cvc sll 2 <data/prepared/order_65-128/polbooks.txt
```
