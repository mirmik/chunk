set(CHUNK_LIB_SOURCES
    src/apply_chunk_v2.cpp
    src/chunk_help.cpp
    src/symbols.cpp
    src/symbols_py.cpp
)

set(CHUNK_LIB_HEADERS
    src/apply_chunk_v2.h
    src/chunk_help.h
    src/symbols.h
)

add_library(chunklib
    ${CHUNK_LIB_SOURCES}
    ${CHUNK_LIB_HEADERS}
)

target_include_directories(chunklib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

if (HAS_LIBSTDCXXFS)
    target_link_libraries(chunklib PUBLIC stdc++fs)
endif()

add_executable(chunk src/main.cpp)
target_link_libraries(chunk PRIVATE chunklib)

if (HAS_LIBSTDCXXFS)
    target_link_libraries(chunk PRIVATE stdc++fs)
endif()

file(GLOB CHUNK_TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp)

add_executable(chunk_tests ${CHUNK_TEST_SOURCES})
target_link_libraries(chunk_tests chunklib)
target_include_directories(chunk_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
)

add_test(NAME chunk_tests COMMAND chunk_tests)
