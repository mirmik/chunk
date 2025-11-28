<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/command.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&nbsp;once<br>
<br>
#include&nbsp;&lt;string&gt;<br>
#include&nbsp;&lt;vector&gt;<br>
<br>
class&nbsp;Command<br>
{<br>
public:<br>
&nbsp;&nbsp;&nbsp;&nbsp;virtual&nbsp;~Command()&nbsp;=&nbsp;default;<br>
&nbsp;&nbsp;&nbsp;&nbsp;virtual&nbsp;void&nbsp;execute(std::vector&lt;std::string&gt;&nbsp;&amp;lines)&nbsp;=&nbsp;0;<br>
};<br>
<br>
<!-- END SCAT CODE -->
</body>
</html>
