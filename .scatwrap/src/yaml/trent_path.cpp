<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/yaml/trent_path.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include&nbsp;&quot;trent_path.h&quot;<br>
<br>
<br>
std::string&nbsp;nos::to_string(const&nbsp;nos::trent_path&nbsp;&amp;path)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::vector&lt;std::string&gt;&nbsp;svec;<br>
&nbsp;&nbsp;&nbsp;&nbsp;for&nbsp;(const&nbsp;auto&nbsp;&amp;node&nbsp;:&nbsp;path)<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(node.is_string)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;svec.push_back(node.str);<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;else<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;svec.push_back(std::to_string(node.i32));<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;nos::join(svec,&nbsp;'/');<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
