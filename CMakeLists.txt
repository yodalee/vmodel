cmake_minimum_required(VERSION 3.20)
project(vmodel LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

find_package(verilator HINTS $ENV{VERILATOR_ROOT} REQUIRED)

set(RTL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/rtl)

add_subdirectory(vmodel)
add_subdirectory(verilog)
add_subdirectory(sim/validready)

# Build all requested binaries with the default target
add_custom_target(all_bins ALL
    DEPENDS sim_cpptop sim_testbench sim_cppsplit test_vint test_shim
)
