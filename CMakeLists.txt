cmake_minimum_required(VERSION 3.20)
project("CVC Approximation"
    LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # export compile_commands.json file
set(CMAKE_CXX_STANDARD_REQUIRED   ON)  # require that the C++ version be specified
set(CMAKE_CXX_STANDARD            20)  # use C++20
set(CMAKE_CXX_EXTENSIONS          OFF) # stick to ISO Standard C++; disable non-standard compiler-specific extensions to the language


## Create the build targets

add_library(simple_graph simple_graph.h simple_graph.cxx)
add_executable(min_cvc min_cvc.cxx)
target_link_libraries(min_cvc PRIVATE simple_graph)

add_library(ordering ordering.h ordering.cxx)
add_library(ilst ilst.h ilst.cxx)
add_executable(approx_cvc approx_cvc.cxx)
target_link_libraries(approx_cvc
    PRIVATE
        simple_graph
        ordering
        ilst
)

set(ALL_TARGETS
    simple_graph
    min_cvc
    ordering
    ilst approx_cvc
)

## Set up compiler warnings

if(MSVC)
    set(WARNINGS
        /W4 # Baseline reasonable warnings
        /w14242 # 'identifier': conversion from 'type1' to 'type2', possible loss of data
        /w14254 # 'operator': conversion from 'type1:field_bits' to 'type2:field_bits', possible loss of data
        /w14263 # 'function': member function does not override any base class virtual member function
        /w14265 # 'classname': class has virtual functions, but destructor is not virtual instances of this class may not be destructed correctly
        /w14287 # 'operator': unsigned/negative constant mismatch
        /we4289 # nonstandard extension used: 'variable': loop control variable declared in the for-loop is used outside the for-loop scope
        /w14296 # 'operator': expression is always 'boolean_value'
        /w14311 # 'variable': pointer truncation from 'type1' to 'type2'
        /w14545 # expression before comma evaluates to a function which is missing an argument list
        /w14546 # function call before comma missing argument list
        /w14547 # 'operator': operator before comma has no effect; expected operator with side-effect
        /w14549 # 'operator': operator before comma has no effect; did you intend 'operator'?
        /w14555 # expression has no effect; expected expression with side- effect
        /w14619 # pragma warning: there is no warning number 'number'
        /w14640 # Enable warning on thread un-safe static member initialization
        /w14826 # Conversion from 'type1' to 'type2' is sign-extended. This may cause unexpected runtime behavior.
        /w14905 # wide string literal cast to 'LPSTR'
        /w14906 # string literal cast to 'LPWSTR'
        /w14928 # illegal copy-initialization; more than one user-defined conversion has been implicitly applied
        /permissive- # standards conformance mode for MSVC compiler.
    )
else()
    set(CLANG_WARNINGS
        -Wall
        -pedantic-errors # error if non-standard C++ is used
        -Wextra # reasonable and standard
        -Wshadow # warn the user if a variable declaration shadows one from a parent context
        -Wnon-virtual-dtor # warn the user if a class with virtual functions has a non-virtual destructor. This helps catch hard to track down memory errors
        -Wold-style-cast # warn for c-style casts
        -Wcast-align # warn for potential performance problem casts
        -Wunused # warn on anything being unused
        -Woverloaded-virtual # warn if you overload (not override) a virtual function
        -Wconversion # warn on type conversions that may lose data
        -Wsign-conversion # warn on sign conversions
        -Wnull-dereference # warn if a null dereference is detected
        -Wdouble-promotion # warn if float is implicit promoted to double
        -Wformat=2 # warn on security issues around functions that format output (ie printf)
        -Wimplicit-fallthrough # warn on statements that fallthrough without an explicit annotation
    )
    if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
        set(WARNINGS ${CLANG_WARNINGS})
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(WARNINGS
            ${CLANG_WARNINGS}
            -Wmisleading-indentation # warn if indentation implies blocks where blocks do not exist
            -Wduplicated-cond # warn if if / else chain has duplicated conditions
            -Wduplicated-branches # warn if if / else branches have duplicated code
            -Wlogical-op # warn about logical operations being used where bitwise were probably wanted
            -Wuseless-cast # warn if you perform a cast to the same type
        )
    endif()
endif()

foreach(target ${ALL_TARGETS})
    target_compile_options(${target}
        INTERFACE $<$<COMPILE_LANGUAGE:CXX>:${WARNINGS}>
    )
endforeach()


## Set up sanitizers

option(ENABLE_SANITIZERS "Enable sanitizers" OFF)
option(USE_MEMORY_SANITIZER "Enable memory sanitizer (available only in Clang); has no effect if ENABLE_SANITIZERS is OFF" OFF)

if (${ENABLE_SANITIZERS})
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
        set(SANITIZERS "undefined,integer,")
        if(${USE_MEMORY_SANITIZER} AND CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
            string(APPEND SANITIZERS "memory")
        else()
            string(APPEND SANITIZERS "address,leak")
        endif()

        foreach(target ${ALL_TARGETS})
            target_compile_options(${target} INTERFACE -fsanitize=${SANITIZERS})
            target_link_options(${target} INTERFACE -fsanitize=${SANITIZERS})
        endforeach()
    elseif(MSVC)
        string(FIND "$ENV{PATH}" "$ENV{VSINSTALLDIR}" index_of_vs_install_dir)
        if("${index_of_vs_install_dir}" STREQUAL "-1")
          message(
            SEND_ERROR
              "Using MSVC sanitizers requires setting the MSVC environment before building the project. Please manually open the MSVC command prompt and rebuild the project."
          )
        endif()

        foreach(target ${ALL_TARGETS})
            target_compile_options(${target} INTERFACE /fsanitize=address /Zi /INCREMENTAL:NO) # MSVC only supports address sanitizer
            target_compile_definitions(${target} INTERFACE _DISABLE_VECTOR_ANNOTATION _DISABLE_STRING_ANNOTATION)
            target_link_options(${target} INTERFACE /INCREMENTAL:NO)
        endforeach()
    endif()
endif()
