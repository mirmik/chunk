<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/commands/insert_before_text.hpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&nbsp;once<br>
<br>
#include&nbsp;&quot;command_base.hpp&quot;<br>
#include&nbsp;&quot;commands/text_utils.h&quot;<br>
<br>
class&nbsp;InsertBeforeTextCommand&nbsp;:&nbsp;public&nbsp;MarkerTextCommand<br>
{<br>
public:<br>
&nbsp;&nbsp;&nbsp;&nbsp;InsertBeforeTextCommand()&nbsp;:&nbsp;MarkerTextCommand(&quot;insert-before-text&quot;)&nbsp;{}<br>
<br>
protected:<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;should_indent_payload()&nbsp;const&nbsp;override&nbsp;{&nbsp;return&nbsp;true;&nbsp;}<br>
&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;apply(std::vector&lt;std::string&gt;&nbsp;&amp;lines,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::size_t&nbsp;begin,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::size_t&nbsp;/*end*/,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;const&nbsp;std::vector&lt;std::string&gt;&nbsp;&amp;payload)&nbsp;override<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;using&nbsp;text_utils::trim;<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::size_t&nbsp;count&nbsp;=&nbsp;payload.size();<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;const&nbsp;std::size_t&nbsp;marker_lines&nbsp;=&nbsp;marker_.size();<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(marker_lines&nbsp;&gt;&nbsp;0&nbsp;&amp;&amp;&nbsp;count&nbsp;&gt;=&nbsp;marker_lines)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;same&nbsp;=&nbsp;true;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;const&nbsp;std::size_t&nbsp;start&nbsp;=&nbsp;count&nbsp;-&nbsp;marker_lines;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;for&nbsp;(std::size_t&nbsp;i&nbsp;=&nbsp;0;&nbsp;i&nbsp;&lt;&nbsp;marker_lines;&nbsp;++i)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(trim(marker_[i])&nbsp;!=&nbsp;trim(payload[start&nbsp;+&nbsp;i]))<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;same&nbsp;=&nbsp;false;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;break;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;}<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;}<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(same)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;count&nbsp;-=&nbsp;marker_lines;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;}<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;auto&nbsp;insert_pos&nbsp;=<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;lines.begin()&nbsp;+&nbsp;static_cast&lt;std::ptrdiff_t&gt;(begin);<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;auto&nbsp;first&nbsp;=&nbsp;payload.begin();<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;auto&nbsp;last&nbsp;=<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;payload.begin()&nbsp;+&nbsp;static_cast&lt;std::ptrdiff_t&gt;(count);<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;lines.insert(insert_pos,&nbsp;first,&nbsp;last);<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
};<br>
<br>
<!-- END SCAT CODE -->
</body>
</html>
