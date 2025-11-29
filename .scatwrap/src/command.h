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
#include&nbsp;&quot;parser.h&quot;<br>
#include&nbsp;&quot;yaml/trent.h&quot;<br>
<br>
#include&nbsp;&lt;iosfwd&gt;<br>
#include&nbsp;&lt;string&gt;<br>
#include&nbsp;&lt;vector&gt;<br>
<br>
class&nbsp;Command<br>
{<br>
public:<br>
&nbsp;&nbsp;&nbsp;&nbsp;explicit&nbsp;Command(std::string&nbsp;name);<br>
&nbsp;&nbsp;&nbsp;&nbsp;virtual&nbsp;~Command()&nbsp;=&nbsp;default;<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;virtual&nbsp;void&nbsp;parse(const&nbsp;nos::trent&nbsp;&amp;tr)&nbsp;=&nbsp;0;<br>
&nbsp;&nbsp;&nbsp;&nbsp;virtual&nbsp;void&nbsp;execute(std::vector&lt;std::string&gt;&nbsp;&amp;lines)&nbsp;=&nbsp;0;<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;const&nbsp;std::string&nbsp;&amp;filepath()&nbsp;const&nbsp;{&nbsp;return&nbsp;filepath_;&nbsp;}<br>
&nbsp;&nbsp;&nbsp;&nbsp;const&nbsp;std::string&nbsp;&amp;command_name()&nbsp;const&nbsp;{&nbsp;return&nbsp;name_;&nbsp;}<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;const&nbsp;std::string&nbsp;&amp;comment()&nbsp;const&nbsp;{&nbsp;return&nbsp;comment_;&nbsp;}<br>
&nbsp;&nbsp;&nbsp;&nbsp;const&nbsp;std::string&nbsp;&amp;language()&nbsp;const&nbsp;{&nbsp;return&nbsp;language_;&nbsp;}<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;set_patch_language(const&nbsp;std::string&nbsp;&amp;lang)&nbsp;{&nbsp;language_&nbsp;=&nbsp;lang;&nbsp;}<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;//&nbsp;Optional&nbsp;hook&nbsp;for&nbsp;additional&nbsp;debug&nbsp;info&nbsp;(e.g.&nbsp;marker&nbsp;preview).<br>
&nbsp;&nbsp;&nbsp;&nbsp;virtual&nbsp;void&nbsp;append_debug_info(std::ostream&nbsp;&amp;os)&nbsp;const;<br>
<br>
protected:<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;filepath_;<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;name_;<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;language_;<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;comment_;<br>
};<br>
<!-- END SCAT CODE -->
</body>
</html>
