# Формат патчей CHUNK

Формат CHUNK задуман c тем, чтобы

* быть достаточно простым для чтения человеком и генерации нейросетью;
* и достаточно строгим, чтобы безопасно модифицировать исходный код.

В этом файле описываем **формат файла и семантику команд**.  
CHUNK использует облегчённый YAML-синтаксис, потому что нейросети достаточно стабильно пишут YAML-документы (в отличие от unified diff).

---

## 1. Структура файла

Корень YAML-документа — **объект** с обязательным полем `operations`:

```yaml
operations:
  - path: src/foo.cpp
    op: replace_text
    ...
  - path: src/foo.py
    op: replace_text
    ...
```

Также опционально, но весьма желательно добавить описание `description` и язык программирования `language` (поддерживаются `c++` и `python`):

```yaml
description: "Краткое описание патча"
language: c++
operations:
  - path: ...
    op: ...
    ...
```

* `operations` — массив операций.
* каждый элемент массива — отдельная операция, причём операции применяются **в порядке** изложения в документе.

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

## 2. Маркеры, whitespace, игнорирование комментариев

Для текстовых и блоковых команд используются поля:

* `marker` — основной фрагмент для поиска;
* `before` — контекст до маркера (опционально);
* `after` — контекст после маркера (опционально).

Последние два — опциональны. Они могут быть использованы для уточнения маркера, если в файле есть повторы.  
Все три поля — многострочные скаляры YAML (`|` / `|-`).

### 2.1. Поиск маркера

При поиске маркера мы **игнорируем различия в отступах, хвостовых пробелах и количестве пустых строк**, чтобы патчи не ломались из-за форматирования различных версий IDE (интегрированная среда разработки) и веб-просмотрщиков, в том числе используемых нейросетями при просмотре интернет-ссылок.

Также **игнорируются комментарии на том языке программирования, что указан в `language`** (`c++` или `python`). Это сделано потому, что нейросеть часто:

* либо «глотает» комментарии при переписывании текста маркера — и тогда их нужно проигнорировать;
* либо, наоборот, включает комментарий в маркер — и это тоже должно корректно обрабатываться.

Правила обработки маркера:

* удаляются пробелы и табы в начале и в конце строки;
* игнорируются пустые строки;
* результирующий диапазон `[begin, end]` рассчитывается по реальным строкам файла, включая выкинутые пустые.

С точки зрения CHUNK следующие маркеры идентичны:

```
         A

B
```

и

```
A
            B
```

поскольку оба приводятся к каноничной форме:

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

* `from-marker` / `marker` / `auto` — выровнять `payload` по отступу маркера  
  (режим по умолчанию, если `options.indent` не указан);
* `none` / `as-is` — оставить отступы `payload` «как есть» в соответствии с правилами YAML.

Базовый отступ берётся:

* для текстовых команд — из первой строки диапазона маркера;
* для языковых команд (`replace_c_style_block`, `replace_py_block`) — из строки с «шапкой» блока (сигнатура функции, `if ...`, `for ...`, `def ...`, `class ...` и так далее).

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
* `payload` — содержимое файла (может быть пустым → пустой файл).

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
* `op: insert_after_text | insert_before_text | replace_text | delete_text | prepend_text | append_text`
* `marker` — обязателен для маркерных операций (`insert_after_text`, `insert_before_text`, `replace_text`, `delete_text`).
* `payload` — обязателен для всех, кроме `delete_text`;

Для `prepend_text` и `append_text` поле `marker` не используется: они просто вставляют `payload` в начало или конец файла.

Дополнительные:

* `before`, `after` — опциональный контекст (указывают, что окружает маркер);
* `comment` — опциональная человекочитаемая заметка к операции;
* `options.indent` — опционально (см. раздел 3).

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

### 5.5. `prepend_text`

Вставить текст в начало файла без указания маркера:

```yaml
operations:
  - path: src/foo.cpp
    op: prepend_text
    payload: |-
      // Этот блок будет вставлен в самое начало файла.
      // ...
```

### 5.6. `append_text`

Вставить текст в конец файла без указания маркера:

```yaml
operations:
  - path: src/foo.cpp
    op: append_text
    payload: |-
      // Этот блок будет добавлен в самый конец файла.
      // ...
```

---

## 6. Команды для C-подобных и Python-блоков

Эти команды работают не с произвольным куском текста, а с **логическими блоками кода** (функции, методы, `if`/`for`-блоки и т.п.).

Поддерживаемые команды:

* `replace_c_style_block`
* `replace_py_block`

Общие поля:

* `path`
* `op`
* `marker` — «шапка» блока (см. ниже);
* `payload` — новый код блока целиком;
* `before`, `after` — опциональный контекст вокруг маркера;
* `comment` — человекочитаемая заметка к операции;
* `options.indent` — режим отступов (как в разделе 3).

### 6.1. C-подобный код: `replace_c_style_block`

Заменяет C-подобный блок кода (функцию, метод, `if`-блок и т.п.), ограниченный фигурными скобками.

* `marker` должен включать «шапку» блока от первой строки сигнатуры до строки с открывающей `{`.  
  При поиске игнорируются комментарии и лишние пробелы так же, как в `replace_text`.
* Область замены — от начала маркера до соответствующей закрывающей `}`.

Пример:

```yaml
operations:
  - path: src/foo.cpp
    op: replace_c_style_block
    comment: "Обновляем реализацию Foo::value в C++"
    marker: |-
      int Foo::value() const
      {
    payload: |-
      int Foo::value() const
      {
          // новая логика
          return 42;
      }
```

### 6.2. Python-подобный код: `replace_py_block`

Команда для Python-блоков, аналогичная `replace_c_style_block`, но вместо фигурных скобок использует двоеточие и отступы.

* `marker` должен включать строку, где находится двоеточие, открывающее блок (`if ...:`, `for ...:`, `while ...:`, `with ...:`, `def ...:`, `class ...:` и т.д.).
* Область замены — от начала маркера до конца блока, то есть до первой строки, чей отступ **не больше**, чем у строки с двоеточием. Пустые строки внутри блока допускаются и не обрывают его.
* Остальные поля (`payload`, `before`, `after`, `options.indent`) работают так же, как у `replace_c_style_block`.

Пример:

```yaml
operations:
  - path: src/foo.py
    op: replace_py_block
    comment: "Обновляем реализацию Foo.run в Python"
    marker: |-
      def run(self):
    payload: |-
      def run(self):
          print("value:", self.value)
          return self.value
```

---

## 7. Пример цельного патча

```yaml
description: "Обновить реализацию значения в C++ и Python"
language: c++

operations:
  - path: src/foo.cpp
    op: replace_c_style_block
    comment: "Обновляем реализацию Foo::value в C++"
    marker: |-
      int Foo::value() const
      {
    payload: |-
      int Foo::value() const
      {
          return 42;
      }

  - path: src/foo.py
    op: replace_py_block
    comment: "Обновляем реализацию Foo.run в Python"
    marker: |-
      def run(self):
    payload: |-
      def run(self):
          print("value:", self.value)
          return self.value
```
