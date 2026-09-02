cmake_minimum_required(VERSION 3.20)
project(vmodel LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

find_package(verilator HINTS $ENV{VERILATOR_ROOT} REQUIRED)

set(RTL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/rtl)

set(VINT_TEST_SRCS
    verilog/dtype/vint_test.cpp
    verilog/dtype/varray_test.cpp
    verilog/dtype/vstruct_test.cpp
    verilog/operation/bit_offset_test.cpp
    verilog/operation/print_indent_test.cpp
    verilog/serialize_test.cpp
)

set(SHIM_TEST_SRCS
    verilog/shim_test.cpp
)

add_library(vmodel INTERFACE)
target_include_directories(vmodel INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(verilog INTERFACE)
target_include_directories(verilog INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(sim/validready)


add_executable(test_vint ${VINT_TEST_SRCS})
target_compile_options(test_vint PRIVATE -Wall)
target_link_libraries(test_vint PRIVATE verilog gtest gtest_main)

add_executable(test_shim ${SHIM_TEST_SRCS})
target_compile_options(test_shim PRIVATE -Wall)
target_link_libraries(test_shim PRIVATE vmodel gtest gtest_main)

# Register the test with CTest
add_test(NAME VintTests COMMAND test_vint)
add_test(NAME ShimTests COMMAND test_shim)

# Build all requested binaries with the default target
add_custom_target(all_bins ALL
    DEPENDS sim_cpptop sim_testbench sim_cppsplit test_vint test_shim
)
