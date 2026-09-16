cmake_minimum_required(VERSION 3.16)

project(SystemVIPC
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Common library implementation
add_library(ipc_lib
    shm_lib.cc
)

target_include_directories(ipc_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Producer
add_executable(producer
    producer.cc
)

target_link_libraries(producer
    PRIVATE
        ipc_lib
)

# Consumer
add_executable(consumer
    consumer.cc
)

target_link_libraries(consumer
    PRIVATE
        ipc_lib
)

# Copy configuration file to the build directory
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.ini
    ${CMAKE_CURRENT_BINARY_DIR}/config.ini
    COPYONLY
)