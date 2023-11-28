## Building

### With CMake

```bash
mkdir build
cmake -S . -B build
cmake --build build
```

### Without CMake

```bash
g++ -std=c++20 -c simple_graph.cxx -o simple_graph.o
g++ -std=c++20 -c ordering.cxx -o ordering.o
g++ -std=c++20 -c ilst.cxx -o ilst.o
g++ -std=c++20 min_cvc.cxx simple_graph.o -o min_cvc
g++ -std=c++20 ilst_cvc.cxx simple_graph.o ordering.o ilst.o -o ilst_cvc
```
