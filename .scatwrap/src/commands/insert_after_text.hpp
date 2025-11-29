<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/commands/insert_after_text.hpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&nbsp;once<br>
<br>
#include&nbsp;&quot;command_base.hpp&quot;<br>
#include&nbsp;&quot;commands/text_utils.h&quot;<br>
<br>
class&nbsp;InsertAfterTextCommand&nbsp;:&nbsp;public&nbsp;MarkerTextCommand<br>
{<br>
public:<br>
&nbsp;&nbsp;&nbsp;&nbsp;InsertAfterTextCommand()&nbsp;:&nbsp;MarkerTextCommand(&quot;insert-after-text&quot;)&nbsp;{}<br>
<br>
protected:<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;should_indent_payload()&nbsp;const&nbsp;override&nbsp;{&nbsp;return&nbsp;true;&nbsp;}<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;apply(std::vector&lt;std::string&gt;&nbsp;&amp;lines,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::size_t&nbsp;begin,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::size_t&nbsp;end,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;const&nbsp;std::vector&lt;std::string&gt;&nbsp;&amp;payload)&nbsp;override<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;(void)begin;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::size_t&nbsp;pos&nbsp;=&nbsp;end&nbsp;+&nbsp;1;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;lines.insert(lines.begin()&nbsp;+&nbsp;static_cast&lt;std::ptrdiff_t&gt;(pos),<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;payload.begin(),<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;payload.end());<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
};<br>
<br>
<!-- END SCAT CODE -->
</body>
</html>
