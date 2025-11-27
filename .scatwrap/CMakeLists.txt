<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>CMakeLists.txt</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
set(CHUNK_LIB_SOURCES<br>
&nbsp;&nbsp;&nbsp;&nbsp;src/apply_chunk_v2.cpp<br>
&nbsp;&nbsp;&nbsp;&nbsp;src/chunk_help.cpp<br>
&nbsp;&nbsp;&nbsp;&nbsp;src/symbols.cpp<br>
&nbsp;&nbsp;&nbsp;&nbsp;src/symbols_py.cpp<br>
)<br>
<br>
set(CHUNK_LIB_HEADERS<br>
&nbsp;&nbsp;&nbsp;&nbsp;src/apply_chunk_v2.h<br>
&nbsp;&nbsp;&nbsp;&nbsp;src/chunk_help.h<br>
&nbsp;&nbsp;&nbsp;&nbsp;src/symbols.h<br>
)<br>
<br>
add_library(chunklib<br>
&nbsp;&nbsp;&nbsp;&nbsp;${CHUNK_LIB_SOURCES}<br>
&nbsp;&nbsp;&nbsp;&nbsp;${CHUNK_LIB_HEADERS}<br>
)<br>
<br>
target_include_directories(chunklib&nbsp;PUBLIC&nbsp;${CMAKE_CURRENT_SOURCE_DIR}/src)<br>
<br>
if&nbsp;(HAS_LIBSTDCXXFS)<br>
&nbsp;&nbsp;&nbsp;&nbsp;target_link_libraries(chunklib&nbsp;PUBLIC&nbsp;stdc++fs)<br>
endif()<br>
<br>
add_executable(chunk&nbsp;src/main.cpp)<br>
target_link_libraries(chunk&nbsp;PRIVATE&nbsp;chunklib)<br>
<br>
if&nbsp;(HAS_LIBSTDCXXFS)<br>
&nbsp;&nbsp;&nbsp;&nbsp;target_link_libraries(chunk&nbsp;PRIVATE&nbsp;stdc++fs)<br>
endif()<br>
<br>
file(GLOB&nbsp;CHUNK_TEST_SOURCES&nbsp;CONFIGURE_DEPENDS&nbsp;${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp)<br>
<br>
add_executable(chunk_tests&nbsp;${CHUNK_TEST_SOURCES})<br>
target_link_libraries(chunk_tests&nbsp;chunklib)<br>
target_include_directories(chunk_tests&nbsp;PRIVATE<br>
&nbsp;&nbsp;&nbsp;&nbsp;${CMAKE_CURRENT_SOURCE_DIR}/src<br>
&nbsp;&nbsp;&nbsp;&nbsp;${CMAKE_SOURCE_DIR}/tests<br>
)<br>
<br>
add_test(NAME&nbsp;chunk_tests&nbsp;COMMAND&nbsp;chunk_tests)<br>
<!-- END SCAT CODE -->
</body>
</html>
