<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/command_parse_helpers.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&nbsp;once<br>
<br>
#include&nbsp;&quot;yaml/trent.h&quot;<br>
<br>
#include&nbsp;&lt;string&gt;<br>
#include&nbsp;&lt;vector&gt;<br>
<br>
namespace&nbsp;command_parse<br>
{<br>
std::string&nbsp;get_scalar(const&nbsp;nos::trent&nbsp;&amp;node,&nbsp;const&nbsp;char&nbsp;*key);<br>
std::vector&lt;std::string&gt;&nbsp;split_scalar_lines(const&nbsp;std::string&nbsp;&amp;text);<br>
bool&nbsp;parse_indent_from_options(const&nbsp;nos::trent&nbsp;&amp;node,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;default_value&nbsp;=&nbsp;true);<br>
}&nbsp;//&nbsp;namespace&nbsp;command_parse<br>
<br>
<!-- END SCAT CODE -->
</body>
</html>
