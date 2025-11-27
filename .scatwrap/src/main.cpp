<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/main.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include&nbsp;&quot;apply_chunk_v2.h&quot;&#13;
#include&nbsp;&quot;chunk_help.h&quot;&#13;
#include&nbsp;&lt;cstring&gt;&#13;
#include&nbsp;&lt;iostream&gt;&#13;
&#13;
int&nbsp;main(int&nbsp;argc,&nbsp;char&nbsp;**argv)&#13;
{&#13;
&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(argc&nbsp;&gt;=&nbsp;2&nbsp;&amp;&amp;&#13;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;(std::strcmp(argv[1],&nbsp;&quot;--help&quot;)&nbsp;==&nbsp;0&nbsp;||&#13;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::strcmp(argv[1],&nbsp;&quot;-h&quot;)&nbsp;==&nbsp;0))&#13;
&nbsp;&nbsp;&nbsp;&nbsp;{&#13;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;print_chunk_help();&#13;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;0;&#13;
&nbsp;&nbsp;&nbsp;&nbsp;}&#13;
&#13;
&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;apply_chunk_main(argc,&nbsp;argv);&#13;
}&#13;
<!-- END SCAT CODE -->
</body>
</html>
