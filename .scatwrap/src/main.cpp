<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/main.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include&nbsp;&quot;apply.h&quot;<br>
#include&nbsp;&quot;help.h&quot;<br>
#include&nbsp;&quot;version.h&quot;<br>
#include&nbsp;&lt;cstring&gt;<br>
#include&nbsp;&lt;iostream&gt;<br>
<br>
int&nbsp;main(int&nbsp;argc,&nbsp;char&nbsp;**argv)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(argc&nbsp;&gt;=&nbsp;2&nbsp;&amp;&amp;&nbsp;(std::strcmp(argv[1],&nbsp;&quot;--help&quot;)&nbsp;==&nbsp;0&nbsp;||<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::strcmp(argv[1],&nbsp;&quot;-h&quot;)&nbsp;==&nbsp;0))<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;print_chunk_help();<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;0;<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(argc&nbsp;&gt;=&nbsp;2&nbsp;&amp;&amp;&nbsp;(std::strcmp(argv[1],&nbsp;&quot;--version&quot;)&nbsp;==&nbsp;0&nbsp;||<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::strcmp(argv[1],&nbsp;&quot;-V&quot;)&nbsp;==&nbsp;0))<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::cout&nbsp;&lt;&lt;&nbsp;&quot;chunk&nbsp;&quot;&nbsp;&lt;&lt;&nbsp;chunk::version_string&nbsp;&lt;&lt;&nbsp;&quot;\n&quot;;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;0;<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(argc&nbsp;&gt;=&nbsp;2&nbsp;&amp;&amp;&nbsp;(std::strcmp(argv[1],&nbsp;&quot;--ref&quot;)&nbsp;==&nbsp;0))<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::cout&nbsp;&lt;&lt;&nbsp;&quot;https://mirmik.github.io/chunk/\n&quot;;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;0;<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;apply_chunk_main(argc,&nbsp;argv);<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
