<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/yaml/yaml.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#ifndef&nbsp;NOS_TRENT_YAML_H&#13;
#define&nbsp;NOS_TRENT_YAML_H&#13;
&#13;
#include&nbsp;&lt;iosfwd&gt;&#13;
#include&nbsp;&quot;trent.h&quot;&#13;
#include&nbsp;&lt;string&gt;&#13;
#include&nbsp;&lt;stdexcept&gt;&#13;
&#13;
namespace&nbsp;nos&#13;
{&#13;
&nbsp;&nbsp;&nbsp;&nbsp;namespace&nbsp;yaml&#13;
&nbsp;&nbsp;&nbsp;&nbsp;{&#13;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;class&nbsp;parse_error&nbsp;:&nbsp;public&nbsp;std::runtime_error&#13;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;{&#13;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;size_t&nbsp;line_&nbsp;=&nbsp;0;&#13;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;size_t&nbsp;column_&nbsp;=&nbsp;0;&#13;
&#13;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;public:&#13;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;parse_error(size_t&nbsp;line,&nbsp;size_t&nbsp;column,&nbsp;const&nbsp;std::string&nbsp;&amp;msg);&#13;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;size_t&nbsp;line()&nbsp;const&nbsp;noexcept&nbsp;{&nbsp;return&nbsp;line_;&nbsp;}&#13;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;size_t&nbsp;column()&nbsp;const&nbsp;noexcept&nbsp;{&nbsp;return&nbsp;column_;&nbsp;}&#13;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;};&#13;
&#13;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;nos::trent&nbsp;parse(const&nbsp;std::string&nbsp;&amp;text);&#13;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;nos::trent&nbsp;parse(const&nbsp;char&nbsp;*text);&#13;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;nos::trent&nbsp;parse_file(const&nbsp;std::string&nbsp;&amp;path);&#13;
&#13;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;print_to(const&nbsp;nos::trent&nbsp;&amp;tr,&nbsp;std::ostream&nbsp;&amp;os);&#13;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;to_string(const&nbsp;nos::trent&nbsp;&amp;tr);&#13;
&nbsp;&nbsp;&nbsp;&nbsp;}&#13;
}&#13;
&#13;
#endif&#13;
<!-- END SCAT CODE -->
</body>
</html>
