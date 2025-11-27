
cmake_minimum_required(VERSION 3.14)
project(chunk LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Все исходники утилиты
file(GLOB CHUNK_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yaml/*.cpp
)

add_executable(chunk ${CHUNK_SOURCES})
target_include_directories(chunk PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yaml
)

if (HAS_LIBSTDCXXFS)
    target_link_libraries(chunk PRIVATE stdc++fs)
endif()
