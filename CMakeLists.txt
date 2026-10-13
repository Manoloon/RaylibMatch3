cmake_minimum_required(VERSION 3.25.0)
project(Matchthree VERSION 0.1.0 LANGUAGES C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(CMAKE_C_COMPILER_ID MATCHES "(GNU|Clang)")
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow)
endif()
find_package(raylib REQUIRED)
# Add the executable target
add_executable(Matchthree main.c )
target_link_libraries(Matchthree raylib)