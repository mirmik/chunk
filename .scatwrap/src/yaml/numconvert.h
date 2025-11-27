<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/yaml/numconvert.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#ifndef&nbsp;NOS_UTIL_NUMCONVERT_H<br>
#define&nbsp;NOS_UTIL_NUMCONVERT_H<br>
<br>
#include&nbsp;&lt;stdint.h&gt;<br>
<br>
char&nbsp;*__nos_ftoa(float&nbsp;f,&nbsp;char&nbsp;*buf,&nbsp;int8_t&nbsp;precision);<br>
char&nbsp;*__nos_dtoa(double&nbsp;f,&nbsp;char&nbsp;*buf,&nbsp;int8_t&nbsp;precision);<br>
char&nbsp;*__nos_ldtoa(long&nbsp;double&nbsp;f,&nbsp;char&nbsp;*buf,&nbsp;int8_t&nbsp;precision);<br>
char&nbsp;*__nos_itoa(int64_t&nbsp;num,&nbsp;char&nbsp;*buf,&nbsp;uint8_t&nbsp;base);<br>
char&nbsp;*__nos_utoa(uint64_t&nbsp;num,&nbsp;char&nbsp;*buf,&nbsp;uint8_t&nbsp;base);<br>
<br>
#endif<br>
<!-- END SCAT CODE -->
</body>
</html>
