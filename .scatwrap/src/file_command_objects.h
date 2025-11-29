<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/file_command_objects.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&nbsp;once<br>
<br>
#include&nbsp;&quot;command.h&quot;<br>
<br>
#include&nbsp;&lt;memory&gt;<br>
<br>
class&nbsp;CreateFileCommand&nbsp;:&nbsp;public&nbsp;Command<br>
{<br>
public:<br>
&nbsp;&nbsp;&nbsp;&nbsp;CreateFileCommand();<br>
&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;parse(const&nbsp;nos::trent&nbsp;&amp;tr)&nbsp;override;<br>
&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;execute(std::vector&lt;std::string&gt;&nbsp;&amp;lines)&nbsp;override;<br>
};<br>
<br>
class&nbsp;DeleteFileCommand&nbsp;:&nbsp;public&nbsp;Command<br>
{<br>
public:<br>
&nbsp;&nbsp;&nbsp;&nbsp;DeleteFileCommand();<br>
&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;parse(const&nbsp;nos::trent&nbsp;&amp;tr)&nbsp;override;<br>
&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;execute(std::vector&lt;std::string&gt;&nbsp;&amp;lines)&nbsp;override;<br>
};<br>
<!-- END SCAT CODE -->
</body>
</html>
