<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>tests/guard/location.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&nbsp;once<br>
<br>
struct&nbsp;guard_location<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;int&nbsp;line;<br>
&nbsp;&nbsp;&nbsp;&nbsp;const&nbsp;char&nbsp;*file;<br>
&nbsp;&nbsp;&nbsp;&nbsp;const&nbsp;char&nbsp;*func;<br>
};<br>
<br>
#define&nbsp;GUARD_CURRENT_LOCATION(name)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\<br>
&nbsp;&nbsp;&nbsp;&nbsp;struct&nbsp;guard_location&nbsp;name&nbsp;=&nbsp;{__LINE__,&nbsp;__FILE__,&nbsp;__func__};<br>
<br>
#define&nbsp;GUARD_CURRENT_LOCATION_INITARGS&nbsp;__LINE__,&nbsp;__FILE__,&nbsp;__func__<br>
<!-- END SCAT CODE -->
</body>
</html>
