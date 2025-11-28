<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/help.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include&nbsp;&quot;help.h&quot;<br>
#include&nbsp;&lt;iostream&gt;<br>
<br>
void&nbsp;print_chunk_help()<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::cout<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;&lt;&nbsp;&quot;chunk&nbsp;-&nbsp;apply&nbsp;chunk_v2&nbsp;patches&nbsp;to&nbsp;a&nbsp;source&nbsp;tree\n&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;&lt;&nbsp;&quot;\n&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;&lt;&nbsp;&quot;Usage:\n&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;&lt;&nbsp;&quot;&nbsp;&nbsp;chunk&nbsp;&lt;patch.yml&gt;\n&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;&lt;&nbsp;&quot;&nbsp;&nbsp;chunk&nbsp;--stdin\n&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;&lt;&nbsp;&quot;&nbsp;&nbsp;chunk&nbsp;--paste\n&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;&lt;&nbsp;&quot;&nbsp;&nbsp;chunk&nbsp;--paste&nbsp;--verbose\n&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;&lt;&nbsp;&quot;&nbsp;&nbsp;chunk&nbsp;--help&nbsp;|&nbsp;-h\n&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;&lt;&nbsp;&quot;&nbsp;&nbsp;chunk&nbsp;--version\n&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;&lt;&nbsp;&quot;\n&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;&lt;&nbsp;&quot;Options:\n&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;&lt;&nbsp;&quot;&nbsp;&nbsp;--stdin&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Read&nbsp;patch&nbsp;from&nbsp;standard&nbsp;input.\n&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;&lt;&nbsp;&quot;&nbsp;&nbsp;--paste&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Read&nbsp;patch&nbsp;from&nbsp;system&nbsp;clipboard.\n&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;&lt;&nbsp;&quot;&nbsp;&nbsp;--verbose&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Print&nbsp;diagnostics&nbsp;while&nbsp;choosing&nbsp;clipboard&nbsp;tool.\n&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;&lt;&nbsp;&quot;&nbsp;&nbsp;--help,&nbsp;-h&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Show&nbsp;this&nbsp;help&nbsp;and&nbsp;exit.\n&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;&lt;&nbsp;&quot;&nbsp;&nbsp;--version&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Show&nbsp;version&nbsp;information&nbsp;and&nbsp;exit.\n&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;&lt;&nbsp;&quot;\n&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;&lt;&nbsp;&quot;Patch&nbsp;format:\n&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;&lt;&nbsp;&quot;&nbsp;&nbsp;Patches&nbsp;must&nbsp;be&nbsp;written&nbsp;in&nbsp;the&nbsp;chunk_v2&nbsp;YAML&nbsp;format.\n&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;&lt;&nbsp;&quot;&nbsp;&nbsp;See&nbsp;the&nbsp;specification:\n&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;&lt;&nbsp;&quot;&nbsp;&nbsp;&nbsp;&nbsp;https://mirmik.github.io/chunk/CHUNK.html\n&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;&lt;&nbsp;&quot;\n&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;&lt;&nbsp;&quot;Clipboard&nbsp;notes:\n&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;&lt;&nbsp;&quot;&nbsp;&nbsp;On&nbsp;Linux&nbsp;/&nbsp;macOS,&nbsp;chunk&nbsp;will&nbsp;try&nbsp;(in&nbsp;this&nbsp;order):\n&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;&lt;&nbsp;&quot;&nbsp;&nbsp;&nbsp;&nbsp;wl-paste\n&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;&lt;&nbsp;&quot;&nbsp;&nbsp;&nbsp;&nbsp;xclip&nbsp;-selection&nbsp;clipboard&nbsp;-o\n&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;&lt;&nbsp;&quot;&nbsp;&nbsp;&nbsp;&nbsp;xsel&nbsp;-b\n&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;&lt;&nbsp;&quot;&nbsp;&nbsp;&nbsp;&nbsp;pbpaste\n&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;&lt;&nbsp;&quot;&nbsp;&nbsp;On&nbsp;Windows&nbsp;it&nbsp;uses:\n&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;&lt;&nbsp;&quot;&nbsp;&nbsp;&nbsp;&nbsp;powershell&nbsp;-NoProfile&nbsp;-Command&nbsp;\&quot;[Console]::OutputEncoding&nbsp;=&nbsp;&quot;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&quot;[System.Text.Encoding]::UTF8;&nbsp;Get-Clipboard&nbsp;-Raw\&quot;\n&quot;;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
