cmake_minimum_required(VERSION 3.12)
project(vertex-cover-finding
    LANGUAGES CXX
)

add_executable(vertex_cover main.cxx)
set_target_properties(vertex_cover
    PROPERTIES
        CXX_STANDARD_REQUIRED ON  # require the C++ version to be specified
        CXX_STANDARD          20  # use C++20
        CXX_EXTENSIONS        OFF # disable compiler-specific extensions
)

# copy the `test_data` directory into the CMake build tree
file(COPY test_data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
