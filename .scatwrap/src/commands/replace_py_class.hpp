<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/commands/replace_py_class.hpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&nbsp;once<br>
<br>
#include&nbsp;&quot;command_base.hpp&quot;<br>
#include&nbsp;&quot;commands/symbol_utils.h&quot;<br>
#include&nbsp;&quot;symbols.h&quot;<br>
<br>
class&nbsp;ReplacePyClassCommand&nbsp;:&nbsp;public&nbsp;RegionReplaceCommand<br>
{<br>
public:<br>
&nbsp;&nbsp;&nbsp;&nbsp;ReplacePyClassCommand()&nbsp;:&nbsp;RegionReplaceCommand(&quot;replace-py-class&quot;)&nbsp;{}<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;parse(const&nbsp;nos::trent&nbsp;&amp;tr)&nbsp;override<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;RegionReplaceCommand::parse(tr);<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;class_text&nbsp;=&nbsp;command_parse::get_scalar(tr,&nbsp;&quot;class&quot;);<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(class_text.empty())<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;throw&nbsp;std::runtime_error(&quot;YAML&nbsp;patch:&nbsp;op&nbsp;'&quot;&nbsp;+&nbsp;command_name()&nbsp;+<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&quot;'&nbsp;requires&nbsp;'class'&nbsp;key&quot;);<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;class_name_&nbsp;=&nbsp;class_text;<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
<br>
protected:<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;find_region(const&nbsp;std::string&nbsp;&amp;text,&nbsp;Region&nbsp;&amp;r)&nbsp;const&nbsp;override<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;PythonSymbolFinder&nbsp;finder(text);<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;finder.find_class(class_name_,&nbsp;r);<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;not_found_error()&nbsp;const&nbsp;override<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;&quot;replace-py-class:&nbsp;class&nbsp;not&nbsp;found:&nbsp;&quot;&nbsp;+&nbsp;class_name_&nbsp;+<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&quot;&nbsp;in&nbsp;file:&nbsp;&quot;&nbsp;+&nbsp;filepath_;<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;invalid_region_error()&nbsp;const&nbsp;override<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;&quot;replace-py-class:&nbsp;invalid&nbsp;region&quot;;<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
<br>
private:<br>
&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;class_name_;<br>
};<br>
<br>
<!-- END SCAT CODE -->
</body>
</html>
