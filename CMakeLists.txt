
cmake_minimum_required(VERSION 3.14)
project(chunk LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-lstdc++fs" HAS_LIBSTDCXXFS)

enable_testing()

# -----------------------------------------------------------------------------
# Windows install prefix
# -----------------------------------------------------------------------------
if (WIN32 AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "$ENV{LOCALAPPDATA}/Programs/chunk"
        CACHE PATH "Install path" FORCE)
endif()

file(GLOB YAML_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yaml/*.cpp
)

file(GLOB CHUNK_CORE_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/languages/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yaml/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/commands/*.cpp
)

file(GLOB CHUNK_HEADERS CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/languages/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/commands/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/commands/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yaml/*.h
)

add_executable(chunk
    ${CMAKE_CURRENT_SOURCE_DIR}/src/app/main.cpp
    ${CHUNK_CORE_SOURCES}
    ${YAML_SOURCES}
    ${CHUNK_HEADERS}
)
target_include_directories(chunk PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yaml
)

include(GNUInstallDirs)
install(TARGETS chunk
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# -----------------------------------------------------------------------------
# Windows shim (chunk.cmd in WindowsApps)
# -----------------------------------------------------------------------------
if (WIN32)
    install(CODE "
        set(wapps \"\$ENV{LOCALAPPDATA}/Microsoft/WindowsApps\")
        file(MAKE_DIRECTORY \"\${wapps}\")
        set(chunk_path \"\$ENV{LOCALAPPDATA}/Programs/chunk/${CMAKE_INSTALL_BINDIR}/chunk.exe\")
if(NOT EXISTS \"\${wapps}/chunk.cmd\")
    file(WRITE \"\${wapps}/chunk.cmd\" \"@echo off\n\\\"\${chunk_path}\\\" %*\")
endif()
    ")
endif()

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
    ${CHUNK_CORE_SOURCES}
    ${YAML_SOURCES}
    ${CHUNK_HEADERS}
)
target_include_directories(chunk_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yaml
    ${CMAKE_CURRENT_SOURCE_DIR}/tests
)


if(WIN32)
    target_link_libraries(chunk_tests PRIVATE ws2_32)
endif()

add_test(NAME chunk_tests COMMAND chunk_tests)
