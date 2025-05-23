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
    ./src/utils/log.c
    ./src/utils/generate_url_theme.c
    ./src/utils/detect_epoch.c
    ./src/utils/code_highlight/escape_code.c
    ./src/utils/code_highlight/highlight_html.c
    ./src/utils/code_highlight/highlight_c.c
    ./src/utils/code_highlight/highlight_code.c
    ./src/utils/code_highlight/highlight_css.c
    ./src/utils/code_highlight/highlight_html.c
    ./src/utils/code_highlight/highlight_js.c
    ./src/utils/code_highlight/highlight_python.c
    ./src/utils/code_highlight/highlight_php.c
    ./src/utils/code_highlight/highlight_basic.c
    ./src/utils/code_highlight/highlight_prolog.c
    ./src/utils/code_highlight/highlight_bat.c
    ./src/utils/code_highlight/highlight_bash.c
    ./src/main.c
    ./src/orchestrator.c
    ./src/web-server/request_handler.c
    ./src/modules/container/container.c
    ./src/modules/menu/menu.c
    ./src/modules/mainbanner/mainbanner.c
    ./src/modules/blog/blog.c
    ./src/modules/page/page.c
    ./src/modules/home_blog/home_blog.c
    ./src/api/google_sheets_api.c
    ./src/api/data_fetcher.c
    ./src/api/route.c
    ./src/api/home_blog_items.c
    ./src/api/page_items.c
)

# Add executable target
add_executable(boat-rudder ${SOURCES})

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
target_link_libraries(boat-rudder ${CURL_LIBRARIES})

# Find Threads package for pthread
find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
    target_compile_options(boat-rudder PRIVATE "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
    target_link_libraries(boat-rudder "${CMAKE_THREAD_LIBS_INIT}")
endif()

# Find OpenSSL package
find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    message(STATUS "OpenSSL found: ${OPENSSL_VERSION}")
    message(STATUS "OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "OpenSSL library SSL: ${OPENSSL_SSL_LIBRARY}")
    message(STATUS "OpenSSL library Crypto: ${OPENSSL_CRYPTO_LIBRARY}")
else()
    message(FATAL_ERROR "Could not find OpenSSL")
endif()

# Include OpenSSL headers
include_directories(${OPENSSL_INCLUDE_DIR})

# Link OpenSSL libraries
target_link_libraries(boat-rudder OpenSSL::SSL OpenSSL::Crypto)

# Add compiler definitions for POSIX compliance
add_definitions(-D_XOPEN_SOURCE=700)

# Add compiler options for warnings
if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
endif()
