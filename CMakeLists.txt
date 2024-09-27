cmake_minimum_required(VERSION 3.10)
project(BoatRudder)

# Set the C standard to C11
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add the executable
add_executable(web_server ./src/web-server/server.c)

# On Unix systems, link with pthread if necessary
if(UNIX)
    find_package(Threads REQUIRED)
    target_link_libraries(web_server PRIVATE Threads::Threads)
endif()
