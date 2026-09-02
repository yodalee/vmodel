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

# Verilated libraries
add_library(verilated_top STATIC)
verilate(verilated_top
    PREFIX VTop
    SOURCES rtl/Top.sv
    INCLUDE_DIRS ${RTL_DIR}
    TOP_MODULE Top
    VERILATOR_ARGS --Wall
    TRACE_FST
)

add_library(verilated_tb_top STATIC)
verilate(verilated_tb_top
    PREFIX Vtb_top
    SOURCES tb_top.sv
    INCLUDE_DIRS ${RTL_DIR}
    TOP_MODULE tb_top
    VERILATOR_ARGS --Wall --timing
    TRACE_FST
)

add_library(verilated_plus1 STATIC)
verilate(verilated_plus1
    PREFIX VPlus1
    SOURCES rtl/Plus1.sv
    INCLUDE_DIRS ${RTL_DIR}
    TOP_MODULE Plus1
    VERILATOR_ARGS --Wall
    TRACE_FST
)

add_library(verilated_foreach STATIC)
verilate(verilated_foreach
    PREFIX VForEach
    SOURCES rtl/ForEach.sv
    INCLUDE_DIRS ${RTL_DIR}
    TOP_MODULE ForEach
    VERILATOR_ARGS --Wall
    TRACE_FST
)

add_library(verilated_repeat STATIC)
verilate(verilated_repeat
    PREFIX VRepeat
    SOURCES rtl/Repeat.sv
    INCLUDE_DIRS ${RTL_DIR}
    TOP_MODULE Repeat
    VERILATOR_ARGS --Wall
    TRACE_FST
)

# C++ simulation executables
add_executable(sim_cpptop sim/sim_cpptop.cpp)
target_compile_options(sim_cpptop PRIVATE -Wall -Wno-sign-compare)
target_link_libraries(sim_cpptop PRIVATE vmodel verilated_top z)

add_executable(sim_testbench sim/sim_testbench.cpp)
target_compile_options(sim_testbench PRIVATE -Wall -Wno-sign-compare)
target_link_libraries(sim_testbench PRIVATE vmodel verilated_tb_top z)

add_executable(sim_cppsplit sim/sim_cppsplit.cpp)
target_compile_options(sim_cppsplit PRIVATE -Wall -Wno-sign-compare)
target_link_libraries(sim_cppsplit PRIVATE vmodel verilated_plus1 verilated_foreach verilated_repeat z)

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
