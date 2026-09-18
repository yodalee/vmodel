set(VINT_TEST_SRCS
    dtype/vint_test.cpp
    dtype/varray_test.cpp
    dtype/vstruct_test.cpp
    operation/bit_offset_test.cpp
    operation/print_indent_test.cpp
    serialize_test.cpp
)

add_library(verilog INTERFACE)
target_include_directories(verilog INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(test_vint ${VINT_TEST_SRCS})
target_compile_options(test_vint PRIVATE -Wall)
target_link_libraries(test_vint PRIVATE verilog gtest gtest_main)

add_test(NAME VintTests COMMAND test_vint)