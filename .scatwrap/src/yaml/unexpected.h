<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/yaml/unexpected.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#ifndef&nbsp;NOS_UTIL_UNEXPECTED_H<br>
#define&nbsp;NOS_UTIL_UNEXPECTED_H<br>
<br>
namespace&nbsp;nos&nbsp;<br>
{<br>
	template&nbsp;&lt;class&nbsp;Err&gt;<br>
	class&nbsp;unexpected&nbsp;<br>
	{<br>
		Err&nbsp;_error;<br>
<br>
	public:<br>
		Err&nbsp;error()&nbsp;const&nbsp;{&nbsp;return&nbsp;_error;&nbsp;}<br>
		unexpected(const&nbsp;Err&amp;&nbsp;err)&nbsp;:&nbsp;_error(err)&nbsp;{}<br>
	};<br>
}<br>
<br>
#endif<br>
<!-- END SCAT CODE -->
</body>
</html>
