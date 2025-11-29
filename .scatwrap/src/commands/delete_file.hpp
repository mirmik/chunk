<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/commands/delete_file.hpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&nbsp;once<br>
<br>
#include&nbsp;&quot;command.h&quot;<br>
#include&nbsp;&quot;command_parse_helpers.h&quot;<br>
<br>
class&nbsp;DeleteFileCommand&nbsp;:&nbsp;public&nbsp;Command<br>
{<br>
public:<br>
&nbsp;&nbsp;&nbsp;&nbsp;DeleteFileCommand()&nbsp;:&nbsp;Command(&quot;delete-file&quot;)&nbsp;{}<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;parse(const&nbsp;nos::trent&nbsp;&amp;tr)&nbsp;override<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;filepath_&nbsp;=&nbsp;command_parse::get_scalar(tr,&nbsp;&quot;path&quot;);<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(filepath_.empty())<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;throw&nbsp;std::runtime_error(<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&quot;YAML&nbsp;patch:&nbsp;op&nbsp;'delete-file'&nbsp;requires&nbsp;'path'&nbsp;key&quot;);<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;comment_&nbsp;=&nbsp;command_parse::get_scalar(tr,&nbsp;&quot;comment&quot;);<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;execute(std::vector&lt;std::string&gt;&nbsp;&amp;lines)&nbsp;override<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;(void)lines;<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
};<br>
<br>
<!-- END SCAT CODE -->
</body>
</html>
