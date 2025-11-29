<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/commands/delete_text.hpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&nbsp;once<br>
<br>
#include&nbsp;&quot;command_base.hpp&quot;<br>
#include&nbsp;&quot;commands/text_utils.h&quot;<br>
<br>
class&nbsp;DeleteTextCommand&nbsp;:&nbsp;public&nbsp;MarkerTextCommand<br>
{<br>
public:<br>
&nbsp;&nbsp;&nbsp;&nbsp;DeleteTextCommand()&nbsp;:&nbsp;MarkerTextCommand(&quot;delete-text&quot;)&nbsp;{}<br>
<br>
private:<br>
&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;apply(std::vector&lt;std::string&gt;&nbsp;&amp;lines,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::size_t&nbsp;begin,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::size_t&nbsp;end,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;const&nbsp;std::vector&lt;std::string&gt;&nbsp;&amp;payload)&nbsp;override<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;(void)payload;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;auto&nbsp;it_begin&nbsp;=&nbsp;lines.begin()&nbsp;+&nbsp;static_cast&lt;std::ptrdiff_t&gt;(begin);<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;auto&nbsp;it_end&nbsp;=&nbsp;lines.begin()&nbsp;+&nbsp;static_cast&lt;std::ptrdiff_t&gt;(end&nbsp;+&nbsp;1);<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;lines.erase(it_begin,&nbsp;it_end);<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
};<br>
<br>
<!-- END SCAT CODE -->
</body>
</html>
