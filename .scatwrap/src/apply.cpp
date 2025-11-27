<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/apply.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include&nbsp;&quot;apply.h&quot;<br>
<br>
#include&nbsp;&quot;runner.h&quot;<br>
#include&nbsp;&quot;section.h&quot;<br>
<br>
#include&nbsp;&lt;fstream&gt;<br>
#include&nbsp;&lt;iostream&gt;<br>
#include&nbsp;&lt;sstream&gt;<br>
<br>
int&nbsp;apply_chunk_main(int&nbsp;argc,&nbsp;char&nbsp;**argv)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(argc&nbsp;&lt;&nbsp;2)<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::cerr&nbsp;&lt;&lt;&nbsp;&quot;Usage:&nbsp;chunk&nbsp;&lt;patchfile&gt;\n&quot;;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;1;<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::ifstream&nbsp;fin(argv[1],&nbsp;std::ios::binary);<br>
&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(!fin)<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::cerr&nbsp;&lt;&lt;&nbsp;&quot;cannot&nbsp;open&nbsp;patch&nbsp;file:&nbsp;&quot;&nbsp;&lt;&lt;&nbsp;argv[1]&nbsp;&lt;&lt;&nbsp;&quot;\n&quot;;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;1;<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::ostringstream&nbsp;oss;<br>
&nbsp;&nbsp;&nbsp;&nbsp;oss&nbsp;&lt;&lt;&nbsp;fin.rdbuf();<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;text&nbsp;=&nbsp;oss.str();<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;try<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;auto&nbsp;sections&nbsp;=&nbsp;parse_yaml_patch_text(text);<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;apply_sections(sections);<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
&nbsp;&nbsp;&nbsp;&nbsp;catch&nbsp;(const&nbsp;std::exception&nbsp;&amp;e)<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::cerr&nbsp;&lt;&lt;&nbsp;&quot;error&nbsp;while&nbsp;applying&nbsp;patch:&nbsp;&quot;&nbsp;&lt;&lt;&nbsp;e.what()&nbsp;&lt;&lt;&nbsp;&quot;\n&quot;;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;1;<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;0;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
