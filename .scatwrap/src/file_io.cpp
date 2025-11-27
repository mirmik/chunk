<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/file_io.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include&nbsp;&quot;file_io.h&quot;<br>
<br>
#include&nbsp;&lt;fstream&gt;<br>
#include&nbsp;&lt;stdexcept&gt;<br>
<br>
std::vector&lt;std::string&gt;&nbsp;read_file_lines(const&nbsp;std::filesystem::path&nbsp;&amp;p)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::ifstream&nbsp;in(p);<br>
&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(!in)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;throw&nbsp;std::runtime_error(&quot;cannot&nbsp;open&nbsp;file:&nbsp;&quot;&nbsp;+&nbsp;p.string());<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::vector&lt;std::string&gt;&nbsp;out;<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;line;<br>
&nbsp;&nbsp;&nbsp;&nbsp;while&nbsp;(std::getline(in,&nbsp;line))<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;out.push_back(line);<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;out;<br>
}<br>
<br>
void&nbsp;write_file_lines(const&nbsp;std::filesystem::path&nbsp;&amp;p,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;const&nbsp;std::vector&lt;std::string&gt;&nbsp;&amp;lines)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::ofstream&nbsp;out(p,&nbsp;std::ios::trunc);<br>
&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(!out)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;throw&nbsp;std::runtime_error(&quot;cannot&nbsp;write&nbsp;file:&nbsp;&quot;&nbsp;+&nbsp;p.string());<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;for&nbsp;(const&nbsp;auto&nbsp;&amp;s&nbsp;:&nbsp;lines)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;out&nbsp;&lt;&lt;&nbsp;s&nbsp;&lt;&lt;&nbsp;&quot;\n&quot;;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
