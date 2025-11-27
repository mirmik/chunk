<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/yaml/error.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#ifndef&nbsp;NOS_ERROR_H&#13;
#define&nbsp;NOS_ERROR_H&#13;
&#13;
#include&nbsp;&lt;string&gt;&#13;
&#13;
namespace&nbsp;nos&#13;
{&#13;
&nbsp;&nbsp;&nbsp;&nbsp;void&nbsp;fatal_handle(const&nbsp;char&nbsp;*msg);&#13;
&#13;
&nbsp;&nbsp;&nbsp;&nbsp;class&nbsp;errstring&nbsp;:&nbsp;public&nbsp;std::string&#13;
&nbsp;&nbsp;&nbsp;&nbsp;{&#13;
&nbsp;&nbsp;&nbsp;&nbsp;public:&#13;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;errstring(const&nbsp;char&nbsp;*msg)&nbsp;:&nbsp;std::string(msg)&nbsp;{}&#13;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;errstring(const&nbsp;std::string&nbsp;&amp;msg)&nbsp;:&nbsp;std::string(msg)&nbsp;{}&#13;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;~errstring()&nbsp;=&nbsp;default;&#13;
&nbsp;&nbsp;&nbsp;&nbsp;};&#13;
}&#13;
&#13;
#endif&#13;
<!-- END SCAT CODE -->
</body>
</html>
