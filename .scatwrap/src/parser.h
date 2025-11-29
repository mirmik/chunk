<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/parser.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&nbsp;once<br>
<br>
#include&nbsp;&lt;memory&gt;<br>
#include&nbsp;&lt;sstream&gt;<br>
#include&nbsp;&lt;string&gt;<br>
#include&nbsp;&lt;vector&gt;<br>
<br>
class&nbsp;Command;<br>
std::vector&lt;std::unique_ptr&lt;Command&gt;&gt;&nbsp;parse_yaml_patch_text(const&nbsp;std::string&nbsp;&amp;text);<br>
<!-- END SCAT CODE -->
</body>
</html>
