# СHUNK. "Не очень строгий" формат патчей исходного кода

Формат CHUNK задуман c тем, чтобы

* быть достаточно простым для чтения человеком и генерации нейросетью;
* и достаточно строгим, чтобы безопасно модифицировать исходный код.

А почему? А потому, что unified diff нейросеть генерирует - такое себе...

---

[Формат патчей CHUNK](CHUNK_ru.md)

[Утилита chunk для применения патчей](https://github.com/mirmik/chunk)

---

Как этим пользоваться? 

Сбрасываете нейросети ссылку на спецификацию формата (можете добавить в персонализацию, лучше бросить файл в файлы проекта, если среда позволяет). Сбрасываете тексты исходного кода своего софта и относительные пути к файлам. И получаете ответ в форме легкоприменимом с помощью утилиты `chunk`:

```bash
chunk --paste
```

И не ползаете по коду, силясь понять, куда вставлять присланную нейросетью портянку.

# CHUNK: a “not-too-strict” source code patch format

The CHUNK format is designed to

* be simple enough for a human to read and for a neural network to generate;
* and strict enough to safely modify source code.

Why? Because when a neural network generates a unified diff, the result is… not exactly great.

---

[CHUNK patch format](CHUNK_en.md)

[`chunk` utility for applying patches](https://github.com/mirmik/chunk)

---

How do you use it?

You give the neural network a link to the format specification (you can add it to personalization; even better, put the file into the project files if your environment allows that).
You send it the source code of your software and the relative paths to the files.
And you get a response in a form that can be easily applied with the `chunk` utility:

```bash
chunk --paste
```

So you no longer have to crawl through the code trying to figure out where exactly to shove the giant blob of text the neural network has sent you.
