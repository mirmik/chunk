<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/file_io.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&nbsp;once<br>
<br>
#include&nbsp;&lt;filesystem&gt;<br>
#include&nbsp;&lt;string&gt;<br>
#include&nbsp;&lt;vector&gt;<br>
<br>
std::vector&lt;std::string&gt;&nbsp;read_file_lines(const&nbsp;std::filesystem::path&nbsp;&amp;p);<br>
void&nbsp;write_file_lines(const&nbsp;std::filesystem::path&nbsp;&amp;p,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;const&nbsp;std::vector&lt;std::string&gt;&nbsp;&amp;lines);<br>
<!-- END SCAT CODE -->
</body>
</html>
