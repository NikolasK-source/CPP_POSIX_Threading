cmake_minimum_required(VERSION 3.16.3 FATAL_ERROR)

file(GLOB lib_SRC "*.cpp")

target_include_directories(${Target} PRIVATE _internal_header)

target_sources(${Target} PRIVATE ${lib_SRC})
target_link_libraries(${Target} INTERFACE pthread)
