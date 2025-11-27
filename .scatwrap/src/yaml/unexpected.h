<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/yaml/unexpected.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#ifndef&nbsp;NOS_UTIL_UNEXPECTED_H&#13;
#define&nbsp;NOS_UTIL_UNEXPECTED_H&#13;
&#13;
namespace&nbsp;nos&nbsp;&#13;
{&#13;
	template&nbsp;&lt;class&nbsp;Err&gt;&#13;
	class&nbsp;unexpected&nbsp;&#13;
	{&#13;
		Err&nbsp;_error;&#13;
&#13;
	public:&#13;
		Err&nbsp;error()&nbsp;const&nbsp;{&nbsp;return&nbsp;_error;&nbsp;}&#13;
		unexpected(const&nbsp;Err&amp;&nbsp;err)&nbsp;:&nbsp;_error(err)&nbsp;{}&#13;
	};&#13;
}&#13;
&#13;
#endif&#13;
<!-- END SCAT CODE -->
</body>
</html>
