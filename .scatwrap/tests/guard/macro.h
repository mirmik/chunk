<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>tests/guard/macro.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&nbsp;once<br>
<br>
#define&nbsp;GUARD_STRINGIFY(...)&nbsp;#__VA_ARGS__<br>
#define&nbsp;GUARD_STRINGIFY2(...)&nbsp;GUARD_STRINGIFY_IMPL(__VA_ARGS__)<br>
<br>
#define&nbsp;GUARD_CONCAT(a,&nbsp;b)&nbsp;a##b<br>
#define&nbsp;GUARD_CONCAT2(a,&nbsp;b)&nbsp;GUARD_CONCAT(a,&nbsp;b)<br>
<!-- END SCAT CODE -->
</body>
</html>
