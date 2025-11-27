<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/yaml/error.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#ifndef&nbsp;NOS_ERROR_H<br>
#define&nbsp;NOS_ERROR_H<br>
<br>
#include&nbsp;&lt;string&gt;<br>
<br>
namespace&nbsp;nos<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;fatal_handle(const&nbsp;char&nbsp;*msg);<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;class&nbsp;errstring&nbsp;:&nbsp;public&nbsp;std::string<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;public:<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;errstring(const&nbsp;char&nbsp;*msg)&nbsp;:&nbsp;std::string(msg)&nbsp;{}<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;errstring(const&nbsp;std::string&nbsp;&amp;msg)&nbsp;:&nbsp;std::string(msg)&nbsp;{}<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;~errstring()&nbsp;=&nbsp;default;<br>
&nbsp;&nbsp;&nbsp;&nbsp;};<br>
}<br>
<br>
#endif<br>
<!-- END SCAT CODE -->
</body>
</html>
