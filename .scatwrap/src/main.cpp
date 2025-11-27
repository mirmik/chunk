<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/main.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include&nbsp;&quot;apply_chunk_v2.h&quot;<br>
#include&nbsp;&quot;chunk_help.h&quot;<br>
#include&nbsp;&lt;cstring&gt;<br>
#include&nbsp;&lt;iostream&gt;<br>
<br>
int&nbsp;main(int&nbsp;argc,&nbsp;char&nbsp;**argv)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(argc&nbsp;&gt;=&nbsp;2&nbsp;&amp;&amp;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;(std::strcmp(argv[1],&nbsp;&quot;--help&quot;)&nbsp;==&nbsp;0&nbsp;||<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::strcmp(argv[1],&nbsp;&quot;-h&quot;)&nbsp;==&nbsp;0))<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;print_chunk_help();<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;0;<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;apply_chunk_main(argc,&nbsp;argv);<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
