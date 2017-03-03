project(daemon)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(./src SRC_LIST)

find_package (ZLIB)
find_package (Threads)

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries (daemon ${CMAKE_THREAD_LIBS_INIT} z)
