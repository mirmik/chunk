
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

include(GNUInstallDirs)
install(TARGETS chunk
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if (HAS_LIBSTDCXXFS)
    target_link_libraries(chunk PRIVATE stdc++fs)
endif()

add_executable(chunk_tests
    tests/apply.cpp          
    tests/apply_yaml.cpp  
    tests/main.cpp     
    tests/symbols_py.cpp
    tests/apply_symbols.cpp  
    tests/symbols.cpp  
    tests/yaml_parser.cpp
    src/yaml/yaml.cpp
    src/yaml/trent.cpp
    src/yaml/trent_path.cpp
    src/yaml/string_ext.cpp
    src/symbols.cpp
    src/symbols_py.cpp
    src/apply_chunk_v2.cpp
    src/chunk_help.cpp
)
target_include_directories(chunk_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yaml
    ${CMAKE_CURRENT_SOURCE_DIR}/tests
)
