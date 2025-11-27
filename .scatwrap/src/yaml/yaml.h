<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/yaml/yaml.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#ifndef&nbsp;NOS_TRENT_YAML_H<br>
#define&nbsp;NOS_TRENT_YAML_H<br>
<br>
#include&nbsp;&lt;iosfwd&gt;<br>
#include&nbsp;&quot;trent.h&quot;<br>
#include&nbsp;&lt;string&gt;<br>
<br>
namespace&nbsp;nos<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;namespace&nbsp;yaml<br>
&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;nos::trent&nbsp;parse(const&nbsp;std::string&nbsp;&amp;text);<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;nos::trent&nbsp;parse(const&nbsp;char&nbsp;*text);<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;nos::trent&nbsp;parse_file(const&nbsp;std::string&nbsp;&amp;path);<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;print_to(const&nbsp;nos::trent&nbsp;&amp;tr,&nbsp;std::ostream&nbsp;&amp;os);<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;to_string(const&nbsp;nos::trent&nbsp;&amp;tr);<br>
&nbsp;&nbsp;&nbsp;&nbsp;}<br>
}<br>
<br>
#endif<br>
<!-- END SCAT CODE -->
</body>
</html>
