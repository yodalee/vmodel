cmake_minimum_required(VERSION 3.20)
project(vmodel LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
target_include_directories(sim_cpptop PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(sim_cpptop PRIVATE -Wall -Wno-sign-compare)
target_link_libraries(sim_cpptop PRIVATE verilated_top z)

add_executable(sim_testbench sim/sim_testbench.cpp)
target_include_directories(sim_testbench PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(sim_testbench PRIVATE -Wall -Wno-sign-compare)
target_link_libraries(sim_testbench PRIVATE verilated_tb_top z)

add_executable(test_vint ${VINT_TEST_SRCS})
target_include_directories(test_vint PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(test_vint PRIVATE -Wall)

find_package(GTest QUIET)
if (GTest_FOUND)
    target_link_libraries(test_vint PRIVATE GTest::gtest GTest::gtest_main pthread)
else()
    target_link_libraries(test_vint PRIVATE gtest gtest_main pthread)
endif()

# Build all requested binaries with the default target
add_custom_target(all_bins ALL
    DEPENDS sim_cpptop sim_testbench test_vint
)
