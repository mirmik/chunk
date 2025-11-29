<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>tests/guard/example.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#define&nbsp;GUARD_TEST_ENABLE_COLORS<br>
#include&nbsp;&quot;check.h&quot;<br>
#include&nbsp;&quot;guard.h&quot;<br>
#include&nbsp;&lt;stdexcept&gt;<br>
<br>
TEST_CASE(&quot;simple&nbsp;arithmetic&quot;)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;CHECK_EQ(2&nbsp;+&nbsp;2,&nbsp;4);<br>
&nbsp;&nbsp;&nbsp;&nbsp;CHECK_LT(1,&nbsp;10);<br>
}<br>
<br>
TEST_CASE(&quot;exceptions&quot;)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;CHECK_THROWS(throw&nbsp;std::runtime_error(&quot;oops&quot;));<br>
&nbsp;&nbsp;&nbsp;&nbsp;CHECK_THROWS_AS(throw&nbsp;std::runtime_error(&quot;oops&quot;),&nbsp;std::runtime_error);<br>
&nbsp;&nbsp;&nbsp;&nbsp;CHECK_NOTHROW(int&nbsp;x&nbsp;=&nbsp;42;&nbsp;(void)x;);<br>
}<br>
<br>
TEST_CASE(&quot;timeout&nbsp;demo&quot;)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;CHECK_TIMEOUT(<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[]<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;{<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;for&nbsp;(volatile&nbsp;int&nbsp;i&nbsp;=&nbsp;0;&nbsp;i&nbsp;&lt;&nbsp;1000000;&nbsp;++i)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;;<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;}(),<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;1000);<br>
}<br>
<br>
TEST_CASE(&quot;failed&nbsp;test:&nbsp;simple&nbsp;arithmetic&quot;)<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;CHECK_EQ(2&nbsp;+&nbsp;2,&nbsp;4);<br>
&nbsp;&nbsp;&nbsp;&nbsp;CHECK_LT(20,&nbsp;10);<br>
}<br>
GUARD_TEST_MAIN();<br>
<!-- END SCAT CODE -->
</body>
</html>
