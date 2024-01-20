project (unittest CXX)

find_package(GTest REQUIRED)

add_executable(unittest unittest.cpp)

include(../Common.cmake)

target_include_directories(unittest PRIVATE ${CMAKE_SOURCE_DIR})

# Link the test executable with the main library and the test framework/library
target_link_libraries(unittest PRIVATE kaitai_struct_cpp_stl_runtime GTest::GTest GTest::Main)

add_test(NAME unittest COMMAND unittest)
