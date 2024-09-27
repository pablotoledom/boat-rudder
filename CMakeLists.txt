cmake_minimum_required(VERSION 3.10)
project(BoatRudder)

# Set the C standard to C11
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find the OpenSSL library
find_package(OpenSSL REQUIRED)

# Verify that OpenSSL was found
if(NOT OpenSSL_FOUND)
    message(FATAL_ERROR "OpenSSL not found!")
endif()

# Add the first executable (web-server) without SSL support
add_executable(web-server ./src/web-server/server.c)

# Include directories for web-server
target_include_directories(web-server PRIVATE
    ./src/web-server
)

# Link libraries for web-server
target_link_libraries(web-server PRIVATE
    Threads::Threads
)

# Add the second executable (web-server-ssl) with SSL support
add_executable(web-server-ssl ./src/web-server/server-ssl.c)

# Include directories for web-server-ssl
target_include_directories(web-server-ssl PRIVATE
    ${OPENSSL_INCLUDE_DIRS}
    ./src/web-server
)

# Link OpenSSL libraries for web-server-ssl
target_link_libraries(web-server-ssl PRIVATE
    ${OPENSSL_LIBRARIES}
    Threads::Threads
)

# Find Threads package
find_package(Threads REQUIRED)
