<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/command.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include&nbsp;&quot;command.h&quot;<br>
<br>
#include&nbsp;&lt;ostream&gt;<br>
<br>
Command::Command(std::string&nbsp;name)<br>
&nbsp;&nbsp;&nbsp;&nbsp;:&nbsp;name_(std::move(name))<br>
{<br>
}<br>
<br>
void&nbsp;Command::append_debug_info(std::ostream&nbsp;&amp;)&nbsp;const<br>
{<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
