# Minimum CMake version required
cmake_minimum_required(VERSION 3.5)

# Project name and language
project(BoatRudder)

# Set the C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Define the source files
set(SOURCES
    ./src/utils/server_utils.c
    ./src/utils/read_file.c
    ./src/utils/json_parser.c
    ./src/main.c
    ./src/orchestrator.c
    ./src/web-server/request_handler.c
    ./src/modules/container/container.c
    ./src/modules/menu/menu.c
    ./src/modules/slider/slider.c
    ./src/modules/blog_home/blog_home.c
    ./src/api/google_sheets_api.c
    ./src/api/data_fetcher.c
    ./src/api/route.c
)

# Add executable target
add_executable(web-server ${SOURCES})

# Find and link libcurl
find_package(CURL REQUIRED)
target_link_libraries(web-server PRIVATE CURL::libcurl)

# Find and link cJSON
find_library(CJSON_LIB cjson)
target_link_libraries(web-server PRIVATE ${CJSON_LIB})

# Find Threads package for pthread
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Link the pthread library
target_link_libraries(web-server PRIVATE Threads::Threads)

# Add compiler definitions for POSIX compliance
target_compile_definitions(web-server PRIVATE
    _XOPEN_SOURCE=700
)

# Add compiler options for warnings
target_compile_options(web-server PRIVATE -Wall -Wextra)
