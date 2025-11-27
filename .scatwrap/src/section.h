<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/section.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&nbsp;once<br>
<br>
#include&nbsp;&lt;string&gt;<br>
#include&nbsp;&lt;vector&gt;<br>
<br>
struct&nbsp;Section<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;filepath;<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;command;<br>
&nbsp;&nbsp;&nbsp;&nbsp;int&nbsp;a&nbsp;=&nbsp;-1;<br>
&nbsp;&nbsp;&nbsp;&nbsp;int&nbsp;b&nbsp;=&nbsp;-1;<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::vector&lt;std::string&gt;&nbsp;payload;<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::vector&lt;std::string&gt;&nbsp;marker;<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::vector&lt;std::string&gt;&nbsp;before;<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::vector&lt;std::string&gt;&nbsp;after;<br>
&nbsp;&nbsp;&nbsp;&nbsp;int&nbsp;seq&nbsp;=&nbsp;0;<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;arg1;<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;arg2;<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;indent_from_marker&nbsp;=&nbsp;true;<br>
};<br>
<br>
bool&nbsp;is_text_command(const&nbsp;std::string&nbsp;&amp;cmd);<br>
bool&nbsp;is_symbol_command(const&nbsp;std::string&nbsp;&amp;cmd);<br>
std::vector&lt;Section&gt;&nbsp;parse_yaml_patch_text(const&nbsp;std::string&nbsp;&amp;text);<br>
<!-- END SCAT CODE -->
</body>
</html>
