cmake_minimum_required(VERSION 3.12)
project(vertex_cover_research)

add_executable(degeneracy main.cxx)
set_target_properties(degeneracy
    PROPERTIES
        CXX_STANDARD_REQUIRED ON
        CXX_STANDARD 20
        CXX_EXTENSIONS OFF
)
