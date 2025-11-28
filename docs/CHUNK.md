# Формат патчей CHUNK

Формат CHUNK задуман c тем, чтобы

* быть достаточно простым для чтения человеком и генерации нейросетью;
* и достаточно строгим, чтобы безопасно модифицировать исходный код.


Файл описываем **формат файла и семантику команд**.
CHUNK использует облегчённый yaml синтаксис потому что нейросети достаточно стабильно пишут yaml документы (в отличие от unified diff)
## Комментарии и символ `#`
Диалект yaml, который использует CHUNK, **не поддерживает комментарии YAML**.
Это сделано потому, что `#` может присутствовать в python коде, в c++ коде, а объяснять кремниевому мозгу, что символ надо экранировать - себе дороже.
Поэтому символ `#` нигде не имеет специального смысла и всегда считается частью строки. 
Для пояснений к патчу в целом используйте поле `description`, а также поле `comment` для коментария каждой операций.

---

## 1. Структура файла

Корень YAML-документа — **объект** с обязательным полем `operations`:

```yaml
operations:
  - path: src/foo.cpp
    op: replace_text
    ...
  - path: src/foo.py
    op: replace_py_method
    ...
````

Также опционально, но весьма желательно добавить описание description и название языка программирования language (поддерживаются python и c++):

```yaml
description: "Краткое описание патча"
language: python
operations:
  - path: ...
    op: ...
    ...
```

* `operations` — массив операций.
* каждый элемент массива — отдельная операция, причём операции применяются в порядке изложения в документе.

Кроме этого, у каждой операции можно задать необязательное поле `comment` с произвольным описанием:

```yaml
operations:
  - path: src/foo.cpp
    op: replace_text
    comment: "Локальная заметка к операции"
    ...
```

Поле `comment` не влияет на применение патча и служит только для пояснений.

---

## 2. Маркеры, whitespace, игнорирование комментариев.

Для текстовых команд используются поля:

* `marker` — основной фрагмент для поиска;
* `before` — контекст до маркера (опционально);
* `after` — контекст после маркера (опционально).

Последние две опциональны. Могут быть использованы для уточнения маркера, если в файле есть повторы.
Все три поля — многострочные скаляры YAML (`|` / `|-`).

**Поиск маркера:**

При поиске маркера мы **игнорируем различия в отступах, хвостовых пробелах и количестве пустых строк**, чтобы патчи не ломались из-за форматирования различных версий ide и вэбпросмотрщиков, в том числе используемых нейросетями при просмотре интернет ссылок. Также **игнорируются комментарии на том языке программирования, что был передан в language**. Это сделано потому, что нейросеть любит глотать коментарии при переписывании текста маркера.

Правила обработки маркера:
* удаляются пробелы и табы в начале и в конце строки;
* игнорируются пустые строки;
* результирующий диапазон `[begin, end]` расчитывается по реальным строкам файла, включая выкинутые пустые.

С точки зрения CHUNK следующие маркеры идентичны
```
         A

B
```
```
A
            B
```
поскольку оба приводятся к каноничной форме 

```
A
B
```

---

## 3. Отступы `payload`

Отступы вставляемого текста по умолчанию выравниваются по идентации начала маркера, но это поведение можно изменить:

```yaml
operations:
  - path: src/foo.cpp
    op: replace_text
    marker: |-
      int value() const;
    payload: |-
      int value() const noexcept;
    options:
      indent: <mode>
```

Режимы:

* `from-marker` / `marker` / `auto` — выровнять `payload` по отступу маркера/символа
  (режим по умолчанию, если `options.indent` не указан);
* `none` / `as-is` — оставить отступы `payload` "как есть" в соответствии с правилами yaml.

Базовый отступ берётся:

* для текстовых команд — из первой строки диапазона маркера;
* для C++/Python-команд — из строки объявления класса/метода.

Отступ добавляется только к **непустым** строкам `payload`.

---

## 4. Файловые команды

### 4.1. `create_file`

Создать или перезаписать файл:

```yaml
operations:
  - path: src/hello.cpp
    op: create_file
    payload: |-
      #include <iostream>

      int main() {
          std::cout << "Hello\n";
          return 0;
      }
```

* `path` — целевой файл.
* `payload` — содержимое файла (может быть пустым -> пустой файл).

### 4.2. `delete_file`

Удалить файл:

```yaml
operations:
  - path: src/obsolete.cpp
    op: delete_file
```

---

## 5. Текстовые команды

Работают с произвольным текстом по маркеру.

Обязательные поля операции:

* `path`
* `op: insert_after_text | insert_before_text | replace_text | delete_text`
* `marker`

Дополнительные:

* `before`, `after` — опциональный контекст (указывают, что окружает маркер);
* `payload` — обязателен для всех, кроме `delete_text`;
* `comment` — опциональная человекочитаемая заметка к операции;
* `options.indent` — опционально.

### 5.1. `replace_text`

Заменить фрагмент по маркеру:

```yaml
operations:
  - path: src/foo.cpp
    op: replace_text
    marker: |-
      int value() const;
    payload: |-
      int value() const noexcept;
```

### 5.2. `insert_after_text`

Вставить текст сразу **после** маркера:

```yaml
operations:
  - path: src/foo.cpp
    op: insert_after_text
    marker: |-
      void Foo::init()
      {
          do_stuff();
      }
    payload: |-
      void Foo::log_state() const
      {
          // ...
      }
```

### 5.3. `insert_before_text`

Вставить текст **перед** маркером:

```yaml
operations:
  - path: src/foo.cpp
    op: insert_before_text
    marker: |-
      public:
          void run();
    payload: |-
      public:
          void debug_run();
```

### 5.4. `delete_text`

Удалить фрагмент по маркеру:

```yaml
operations:
  - path: src/foo.cpp
    op: delete_text
    marker: |-
      #include "debug.h"
```

---

## 6. Команды для C++ / Python-символов

Работают с классами и методами, а не с произвольным текстом.

Поддерживаемые команды:

* `replace_cpp_class`
* `replace_cpp_method`
* `replace_py_class`
* `replace_py_method`

Общие поля:

* `path`
* `op`
* `payload` — новый код класса/метода;
* `comment` — опциональная человекочитаемая заметка к операции;
* `options.indent` — режим отступов.

Цель задаётся либо:

* через `class` + `method`,
* либо через `symbol`.

### 6.1. C++: `replace_cpp_class`

```yaml
operations:
  - path: src/foo.cpp
    op: replace_cpp_class
    class: Foo
    payload: |-
      class Foo {
      public:
          int value() const { return 42; }
      };
```

### 6.2. C++: `replace_cpp_method`

```yaml
operations:
  - path: src/foo.cpp
    op: replace_cpp_method
    class: Foo
    method: value
    payload: |-
      int Foo::value() const
      {
          return 42;
      }
```

или короткая форма:

```yaml
operations:
  - path: src/foo.cpp
    op: replace_cpp_method
    symbol: Foo::value
    payload: |-
      int Foo::value() const
      {
          return 42;
      }
```

### 6.3. Python: `replace_py_class`

```yaml
operations:
  - path: src/foo.py
    op: replace_py_class
    class: Foo
    payload: |-
      class Foo:
          def __init__(self):
              self.x = 2

          def answer(self):
              return 42
```

### 6.4. Python: `replace_py_method`

```yaml
operations:
  - path: src/foo.py
    op: replace_py_method
    class: Foo
    method: run
    payload: |-
      def run(self):
          print("running...")
          return 100
```

или:

```yaml
operations:
  - path: src/foo.py
    op: replace_py_method
    symbol: Foo.run
    payload: |-
      def run(self):
          print("running...")
          return 100
```

---

## 7. Пример цельного патча

```yaml
description: "Обновить реализацию значения в C++ и Python"

operations:
  - path: src/foo.cpp
    op: replace_cpp_method
    comment: "Обновляем реализацию Foo::value в C++"
    class: Foo
    method: value
    payload: |-
      int Foo::value() const
      {
          return 42;
      }

  - path: src/foo.py
    op: replace_py_method
    comment: "Обновляем реализацию Foo.run в Python"
    class: Foo
    method: run
    payload: |-
      def run(self):
          print("value:", self.value)
          return self.value
```
