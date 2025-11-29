<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/commands/create_file.hpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&nbsp;once<br>
<br>
#include&nbsp;&quot;command.h&quot;<br>
#include&nbsp;&quot;command_parse_helpers.h&quot;<br>
<br>
class&nbsp;CreateFileCommand&nbsp;:&nbsp;public&nbsp;Command<br>
{<br>
public:<br>
&nbsp;&nbsp;&nbsp;&nbsp;CreateFileCommand()&nbsp;:&nbsp;Command(&quot;create-file&quot;)&nbsp;{}<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;parse(const&nbsp;nos::trent&nbsp;&amp;tr)&nbsp;override<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;filepath_&nbsp;=&nbsp;command_parse::get_scalar(tr,&nbsp;&quot;path&quot;);<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(filepath_.empty())<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;throw&nbsp;std::runtime_error(<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&quot;YAML&nbsp;patch:&nbsp;op&nbsp;'create-file'&nbsp;requires&nbsp;'path'&nbsp;key&quot;);<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;payload_text&nbsp;=&nbsp;command_parse::get_scalar(tr,&nbsp;&quot;payload&quot;);<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;payload_&nbsp;=&nbsp;command_parse::split_scalar_lines(payload_text);<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(payload_.empty())<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;throw&nbsp;std::runtime_error(<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&quot;YAML&nbsp;patch:&nbsp;text&nbsp;op&nbsp;'create-file'&nbsp;for&nbsp;file&nbsp;'&quot;&nbsp;+<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;filepath_&nbsp;+&nbsp;&quot;'&nbsp;requires&nbsp;'payload'&quot;);<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;comment_&nbsp;=&nbsp;command_parse::get_scalar(tr,&nbsp;&quot;comment&quot;);<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;execute(std::vector&lt;std::string&gt;&nbsp;&amp;lines)&nbsp;override<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;lines&nbsp;=&nbsp;payload_;<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
<br>
private:<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::vector&lt;std::string&gt;&nbsp;payload_;<br>
};<br>
<br>
<!-- END SCAT CODE -->
</body>
</html>
