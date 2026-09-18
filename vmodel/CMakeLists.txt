add_library(vmodel INTERFACE)
target_include_directories(vmodel INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(test_shim shim_test.cpp)
target_compile_options(test_shim PRIVATE -Wall)
target_link_libraries(test_shim PRIVATE vmodel gtest gtest_main)

add_test(NAME ShimTests COMMAND test_shim)