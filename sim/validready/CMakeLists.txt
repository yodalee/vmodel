add_library(verilated_top STATIC)
verilate(verilated_top
    PREFIX VTop
    SOURCES ${RTL_DIR}/Top.sv
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
    SOURCES ${RTL_DIR}/Plus1.sv
    INCLUDE_DIRS ${RTL_DIR}
    TOP_MODULE Plus1
    VERILATOR_ARGS --Wall
    TRACE_FST
)

add_library(verilated_foreach STATIC)
verilate(verilated_foreach
    PREFIX VForEach
    SOURCES ${RTL_DIR}/ForEach.sv
    INCLUDE_DIRS ${RTL_DIR}
    TOP_MODULE ForEach
    VERILATOR_ARGS --Wall
    TRACE_FST
)

add_library(verilated_repeat STATIC)
verilate(verilated_repeat
    PREFIX VRepeat
    SOURCES ${RTL_DIR}/Repeat.sv
    INCLUDE_DIRS ${RTL_DIR}
    TOP_MODULE Repeat
    VERILATOR_ARGS --Wall
    TRACE_FST
)

add_executable(sim_validready_cpptop sim_cpptop.cpp)
target_compile_options(sim_validready_cpptop PRIVATE -Wall -Wno-sign-compare)
target_link_libraries(sim_validready_cpptop PRIVATE vmodel verilated_top z)

add_executable(sim_validready_testbench sim_testbench.cpp)
target_compile_options(sim_validready_testbench PRIVATE -Wall -Wno-sign-compare)
target_link_libraries(sim_validready_testbench PRIVATE vmodel verilated_tb_top z)

add_executable(sim_validready_cppsplit sim_cppsplit.cpp)
target_compile_options(sim_validready_cppsplit PRIVATE -Wall -Wno-sign-compare)
target_link_libraries(sim_validready_cppsplit PRIVATE vmodel verilated_plus1 verilated_foreach verilated_repeat z)
