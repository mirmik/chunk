<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>CMakeLists.txt</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
&#13;
cmake_minimum_required(VERSION&nbsp;3.14)&#13;
project(chunk&nbsp;LANGUAGES&nbsp;CXX)&#13;
&#13;
set(CMAKE_CXX_STANDARD&nbsp;17)&#13;
set(CMAKE_CXX_STANDARD_REQUIRED&nbsp;ON)&#13;
&#13;
#&nbsp;Все&nbsp;исходники&nbsp;утилиты&#13;
file(GLOB&nbsp;CHUNK_SOURCES&nbsp;CONFIGURE_DEPENDS&#13;
&nbsp;&nbsp;&nbsp;&nbsp;${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp&#13;
&nbsp;&nbsp;&nbsp;&nbsp;${CMAKE_CURRENT_SOURCE_DIR}/src/yaml/*.cpp&#13;
)&#13;
&#13;
add_executable(chunk&nbsp;${CHUNK_SOURCES})&#13;
target_include_directories(chunk&nbsp;PRIVATE&#13;
&nbsp;&nbsp;&nbsp;&nbsp;${CMAKE_CURRENT_SOURCE_DIR}/src&#13;
&nbsp;&nbsp;&nbsp;&nbsp;${CMAKE_CURRENT_SOURCE_DIR}/src/yaml&#13;
)&#13;
&#13;
if&nbsp;(HAS_LIBSTDCXXFS)&#13;
&nbsp;&nbsp;&nbsp;&nbsp;target_link_libraries(chunk&nbsp;PRIVATE&nbsp;stdc++fs)&#13;
endif()&#13;
&#13;
add_executable(chunk_tests&#13;
&nbsp;&nbsp;&nbsp;&nbsp;tests/apply.cpp&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&#13;
&nbsp;&nbsp;&nbsp;&nbsp;tests/apply_yaml.cpp&nbsp;&nbsp;&#13;
&nbsp;&nbsp;&nbsp;&nbsp;tests/main.cpp&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&#13;
&nbsp;&nbsp;&nbsp;&nbsp;tests/symbols_py.cpp&#13;
&nbsp;&nbsp;&nbsp;&nbsp;tests/apply_symbols.cpp&nbsp;&nbsp;&#13;
&nbsp;&nbsp;&nbsp;&nbsp;tests/symbols.cpp&nbsp;&nbsp;&#13;
&nbsp;&nbsp;&nbsp;&nbsp;tests/yaml_parser.cpp&#13;
&nbsp;&nbsp;&nbsp;&nbsp;src/yaml/yaml.cpp&#13;
&nbsp;&nbsp;&nbsp;&nbsp;src/yaml/trent.cpp&#13;
&nbsp;&nbsp;&nbsp;&nbsp;src/yaml/trent_path.cpp&#13;
&nbsp;&nbsp;&nbsp;&nbsp;src/yaml/string_ext.cpp&#13;
&nbsp;&nbsp;&nbsp;&nbsp;src/symbols.cpp&#13;
&nbsp;&nbsp;&nbsp;&nbsp;src/symbols_py.cpp&#13;
&nbsp;&nbsp;&nbsp;&nbsp;src/apply_chunk_v2.cpp&#13;
&nbsp;&nbsp;&nbsp;&nbsp;src/chunk_help.cpp&#13;
)&#13;
target_include_directories(chunk_tests&nbsp;PRIVATE&#13;
&nbsp;&nbsp;&nbsp;&nbsp;${CMAKE_CURRENT_SOURCE_DIR}/src&#13;
&nbsp;&nbsp;&nbsp;&nbsp;${CMAKE_CURRENT_SOURCE_DIR}/src/yaml&#13;
&nbsp;&nbsp;&nbsp;&nbsp;${CMAKE_CURRENT_SOURCE_DIR}/tests&#13;
)&#13;
<!-- END SCAT CODE -->
</body>
</html>
