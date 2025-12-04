<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/version.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&nbsp;once<br>
<br>
namespace&nbsp;chunk<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;inline&nbsp;constexpr&nbsp;int&nbsp;version_major&nbsp;=&nbsp;0;<br>
&nbsp;&nbsp;&nbsp;&nbsp;inline&nbsp;constexpr&nbsp;int&nbsp;version_minor&nbsp;=&nbsp;3;<br>
&nbsp;&nbsp;&nbsp;&nbsp;inline&nbsp;constexpr&nbsp;int&nbsp;version_patch&nbsp;=&nbsp;0;<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;inline&nbsp;constexpr&nbsp;const&nbsp;char&nbsp;*version_string&nbsp;=&nbsp;&quot;0.3.0&quot;;<br>
}&nbsp;//&nbsp;namespace&nbsp;chunk<br>
<br>
/*&nbsp;CHANGELIST<br>
<br>
0.4.0&nbsp;<br>
&nbsp;&nbsp;&nbsp;&nbsp;Symbolyc&nbsp;api&nbsp;removed.&nbsp;Create&nbsp;file&nbsp;command&nbsp;also&nbsp;create&nbsp;parent&nbsp;directories&nbsp;as&nbsp;needed.<br>
0.3.0&nbsp;<br>
&nbsp;&nbsp;&nbsp;&nbsp;The&nbsp;program&nbsp;no&nbsp;longer&nbsp;writes&nbsp;the&nbsp;result&nbsp;of&nbsp;each&nbsp;command&nbsp;to&nbsp;disk,&nbsp;<br>
&nbsp;&nbsp;&nbsp;&nbsp;but&nbsp;first&nbsp;compiles&nbsp;a&nbsp;new&nbsp;file&nbsp;in&nbsp;its&nbsp;memory&nbsp;and&nbsp;<br>
&nbsp;&nbsp;&nbsp;&nbsp;writes&nbsp;changes&nbsp;to&nbsp;all&nbsp;files&nbsp;at&nbsp;once&nbsp;if&nbsp;the&nbsp;patch&nbsp;was&nbsp;applied&nbsp;successfully.<br>
0.2.0&nbsp;Added&nbsp;append&nbsp;and&nbsp;prepend&nbsp;commands,&nbsp;object&nbsp;refactoring&nbsp;has&nbsp;been&nbsp;performed<br>
0.1.0&nbsp;Initial<br>
<br>
*/<br>
<!-- END SCAT CODE -->
</body>
</html>
