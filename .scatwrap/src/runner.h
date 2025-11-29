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
#pragma&nbsp;once<br>
<br>
#include&nbsp;&quot;command.h&quot;<br>
<br>
#include&nbsp;&lt;cstddef&gt;<br>
#include&nbsp;&lt;memory&gt;<br>
#include&nbsp;&lt;string&gt;<br>
#include&nbsp;&lt;vector&gt;<br>
<br>
struct&nbsp;DryRunStats<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;struct&nbsp;CommandInfo<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;const&nbsp;Command&nbsp;*cmd&nbsp;=&nbsp;nullptr;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;filepath;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::size_t&nbsp;inserted&nbsp;=&nbsp;0;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::size_t&nbsp;erased&nbsp;=&nbsp;0;<br>
&nbsp;&nbsp;&nbsp;&nbsp;};<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::vector&lt;CommandInfo&gt;&nbsp;commands;<br>
};<br>
<br>
DryRunStats&nbsp;apply_sections(const&nbsp;std::vector&lt;std::unique_ptr&lt;Command&gt;&gt;&nbsp;&amp;commands,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;dry_run);<br>
<!-- END SCAT CODE -->
</body>
</html>
