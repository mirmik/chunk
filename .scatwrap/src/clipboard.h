<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/clipboard.h</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#pragma&nbsp;once<br>
<br>
#include&nbsp;&lt;string&gt;<br>
<br>
struct&nbsp;ClipboardReader<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;const&nbsp;char&nbsp;*name;&nbsp;&nbsp;&nbsp;&nbsp;//&nbsp;человекочитаемое&nbsp;имя<br>
&nbsp;&nbsp;&nbsp;&nbsp;const&nbsp;char&nbsp;*command;&nbsp;//&nbsp;команда&nbsp;для&nbsp;popen/_popen<br>
&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;(*is_available)();&nbsp;//&nbsp;опциональная&nbsp;проверка&nbsp;среды,&nbsp;nullptr&nbsp;если&nbsp;не<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;//&nbsp;нужна<br>
};<br>
<br>
bool&nbsp;run_clipboard_reader(const&nbsp;ClipboardReader&nbsp;&amp;reader,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;std::string&nbsp;&amp;out,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;bool&nbsp;verbose);<br>
<br>
bool&nbsp;read_clipboard(std::string&nbsp;&amp;out,&nbsp;bool&nbsp;verbose);<br>
&nbsp;<br>
<!-- END SCAT CODE -->
</body>
</html>
