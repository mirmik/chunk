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
#include&nbsp;&quot;section.h&quot;<br>
#include&nbsp;&quot;yaml/trent.h&quot;<br>
<br>
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
&nbsp;&nbsp;&nbsp;&nbsp;const&nbsp;std::string&nbsp;&amp;filepath()&nbsp;const&nbsp;{&nbsp;return&nbsp;section_.filepath;&nbsp;}<br>
&nbsp;&nbsp;&nbsp;&nbsp;const&nbsp;std::string&nbsp;&amp;command_name()&nbsp;const&nbsp;{&nbsp;return&nbsp;section_.command;&nbsp;}<br>
&nbsp;&nbsp;&nbsp;&nbsp;const&nbsp;Section&nbsp;&amp;data()&nbsp;const&nbsp;{&nbsp;return&nbsp;section_;&nbsp;}<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;set_patch_language(const&nbsp;std::string&nbsp;&amp;lang)&nbsp;{&nbsp;section_.language&nbsp;=&nbsp;lang;&nbsp;}<br>
&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;set_sequence(int&nbsp;seq)&nbsp;{&nbsp;section_.seq&nbsp;=&nbsp;seq;&nbsp;}<br>
&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;load_section(const&nbsp;Section&nbsp;&amp;s)&nbsp;{&nbsp;section_&nbsp;=&nbsp;s;&nbsp;}<br>
<br>
protected:<br>
&nbsp;&nbsp;&nbsp;&nbsp;Section&nbsp;section_;<br>
};<br>
<!-- END SCAT CODE -->
</body>
</html>
