<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>docs/index.md</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#&nbsp;СHUNK.&nbsp;&quot;Не&nbsp;очень&nbsp;строгий&quot;&nbsp;формат&nbsp;патчей&nbsp;исходного&nbsp;кода<br>
<br>
Формат&nbsp;CHUNK&nbsp;задуман&nbsp;c&nbsp;тем,&nbsp;чтобы<br>
<br>
*&nbsp;быть&nbsp;достаточно&nbsp;простым&nbsp;для&nbsp;чтения&nbsp;человеком&nbsp;и&nbsp;генерации&nbsp;нейросетью;<br>
*&nbsp;и&nbsp;достаточно&nbsp;строгим,&nbsp;чтобы&nbsp;безопасно&nbsp;модифицировать&nbsp;исходный&nbsp;код.<br>
<br>
А&nbsp;почему?&nbsp;А&nbsp;потому,&nbsp;что&nbsp;unified&nbsp;diff&nbsp;нейросеть&nbsp;генерирует&nbsp;-&nbsp;такое&nbsp;себе...<br>
<br>
---<br>
<br>
[Формат&nbsp;патчей&nbsp;CHUNK](CHUNK.md)<br>
<br>
[Утилита&nbsp;chunk&nbsp;для&nbsp;применения&nbsp;патчей](https://github.com/mirmik/chunk)<br>
<br>
---<br>
<br>
Как&nbsp;этим&nbsp;пользоваться?&nbsp;<br>
<br>
Сбрасываете&nbsp;нейросети&nbsp;ссылку&nbsp;на&nbsp;спецификацию&nbsp;формата&nbsp;(можете&nbsp;добавить&nbsp;в&nbsp;персонализацию,&nbsp;лучше&nbsp;бросить&nbsp;файл&nbsp;в&nbsp;файлы&nbsp;проекта,&nbsp;если&nbsp;среда&nbsp;позволяет).&nbsp;Сбрасываете&nbsp;тексты&nbsp;исходного&nbsp;кода&nbsp;своего&nbsp;софта&nbsp;и&nbsp;относительные&nbsp;пути&nbsp;к&nbsp;файлам.&nbsp;И&nbsp;получаете&nbsp;ответ&nbsp;в&nbsp;форме&nbsp;легкоприменимом&nbsp;с&nbsp;помощью&nbsp;утилиты&nbsp;`chunk`:<br>
<br>
```bash<br>
chunk&nbsp;--paste<br>
```<br>
<br>
И&nbsp;не&nbsp;ползаете&nbsp;по&nbsp;коду,&nbsp;силясь&nbsp;понять,&nbsp;куда&nbsp;вставлять&nbsp;присланную&nbsp;нейросетью&nbsp;портянку.<br>
<!-- END SCAT CODE -->
</body>
</html>
