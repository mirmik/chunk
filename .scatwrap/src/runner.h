<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/runner.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&nbsp;once<br>
<br>
#include&nbsp;&quot;command.h&quot;<br>
<br>
#include&nbsp;&lt;memory&gt;<br>
#include&nbsp;&lt;vector&gt;<br>
<br>
struct&nbsp;ApplyOptions<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;dry_run&nbsp;=&nbsp;false;<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;ignore_failures&nbsp;=&nbsp;false;<br>
};<br>
<br>
void&nbsp;apply_sections(const&nbsp;std::vector&lt;std::unique_ptr&lt;Command&gt;&gt;&nbsp;&amp;commands,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;const&nbsp;ApplyOptions&nbsp;&amp;options&nbsp;=&nbsp;{});<br>
<!-- END SCAT CODE -->
</body>
</html>
