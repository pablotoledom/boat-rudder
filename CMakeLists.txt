# Minimum CMake version required
cmake_minimum_required(VERSION 2.8.12)

# Project name and language
project(BoatRudder C)

# Set the C standard
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Define the source files
set(SOURCES
    ./src/utils/server_utils.c
    ./src/utils/read_file.c
    ./src/utils/json_parser.c
    ./src/utils/cJSON/cJSON.c
    ./src/utils/print_table.c
    ./src/utils/config_loader.c
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

# Find CURL package
find_package(CURL REQUIRED)

if(CURL_FOUND)
    message(STATUS "CURL found: ${CURL_VERSION_STRING}")
    message(STATUS "CURL include dir: ${CURL_INCLUDE_DIRS}")
    message(STATUS "CURL library: ${CURL_LIBRARIES}")
else()
    message(FATAL_ERROR "Could not find CURL")
endif()

# Include the CURL headers
include_directories(${CURL_INCLUDE_DIRS})

# Link the CURL library
target_link_libraries(web-server ${CURL_LIBRARIES})

# Find Threads package for pthread
find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
    target_compile_options(web-server PRIVATE "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
    target_link_libraries(web-server "${CMAKE_THREAD_LIBS_INIT}")
endif()

# Add compiler definitions for POSIX compliance
add_definitions(-D_XOPEN_SOURCE=700)

# Add compiler options for warnings
if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
endif()