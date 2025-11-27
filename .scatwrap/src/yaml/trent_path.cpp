<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/yaml/trent_path.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include&nbsp;&quot;trent_path.h&quot;&#13;
&#13;
&#13;
std::string&nbsp;nos::to_string(const&nbsp;nos::trent_path&nbsp;&amp;path)&#13;
{&#13;
&nbsp;&nbsp;&nbsp;&nbsp;std::vector&lt;std::string&gt;&nbsp;svec;&#13;
&nbsp;&nbsp;&nbsp;&nbsp;for&nbsp;(const&nbsp;auto&nbsp;&amp;node&nbsp;:&nbsp;path)&#13;
&nbsp;&nbsp;&nbsp;&nbsp;{&#13;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;if&nbsp;(node.is_string)&#13;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;svec.push_back(node.str);&#13;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;else&#13;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;svec.push_back(std::to_string(node.i32));&#13;
&nbsp;&nbsp;&nbsp;&nbsp;}&#13;
&nbsp;&nbsp;&nbsp;&nbsp;return&nbsp;nos::join(svec,&nbsp;'/');&#13;
}&#13;
<!-- END SCAT CODE -->
</body>
</html>
