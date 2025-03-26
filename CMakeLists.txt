cmake_minimum_required(VERSION 3.10)
project(FTP_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add src directory to include path
include_directories(${PROJECT_SOURCE_DIR}/src)

# Add source files
add_executable(ftp_server 
    src/server.cpp
    src/commands.cpp
    src/thread_pool.cpp
) 