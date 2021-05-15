cmake_minimum_required(VERSION 3.19)
project(polymorphic_alloc_testing)

include(FetchContent)
FetchContent_Declare(
        googletest
        # Specify the commit you depend on and update it regularly.
        URL https://github.com/google/googletest/archive/703bd9caab50b139428cea1aaff9974ebee5742e.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(CMAKE_CXX_STANDARD 17)

add_library(polymorphic_alloc_testing
        src/library.h)
target_link_libraries(polymorphic_alloc_testing PUBLIC gmock)

add_executable(library_test
        tests/test_allocator_propagation.cpp)

target_link_libraries(library_test
        polymorphic_alloc_testing
        gmock_main)

