# CHUNK Patch Format

The CHUNK format is designed to:

* be simple enough for a human to read and for a neural network to generate;
* and strict enough to safely modify source code.

In this document we describe the **file format and the command semantics**.
CHUNK uses a lightweight YAML (Yet Another Markup Language) syntax because neural networks tend to generate YAML documents fairly reliably (unlike unified diffs).

---

## Comments and the `#` Character

The YAML dialect used by CHUNK **does not support YAML comments**.

This is because `#` may appear in Python code, in C++ (C plus plus) code, and explaining to a silicon brain that the character must be escaped is more trouble than it is worth.

Therefore the `#` character never has any special meaning and is always treated as part of the line.

To provide explanations for a patch as a whole, use the top-level `description` field, and use the `comment` field for per-operation notes.

---

## 1. File Structure

The root of a YAML document is an **object** (a mapping) with a required `operations` field:

```yaml
operations:
  - path: src/foo.cpp
    op: replace_text
    ...
  - path: src/foo.py
    op: replace_py_method
    ...
```

Additionally, it is optional but highly recommended to add a `description` and the programming `language` (supported values: `python` and `c++`):

```yaml
description: "Short description of the patch"
language: c++
operations:
  - path: ...
    op: ...
    ...
```

* `operations` — an array (list) of operations.
* each array element is a separate operation, and operations are applied in the order they appear in the document.

Each operation may also have an optional `comment` field with an arbitrary description:

```yaml
operations:
  - path: src/foo.cpp
    op: replace_text
    comment: "Local note for this operation"
    ...
```

The `comment` field does not affect how the patch is applied and is only used for human-readable explanations.

---

## 2. Markers, Whitespace, and Ignoring Comments

Text commands use the following fields:

* `marker` — the main fragment to search for;
* `before` — context before the marker (optional);
* `after` — context after the marker (optional).

The last two are optional. They can be used to disambiguate the marker if the file contains repeated fragments.
All three fields are multi-line YAML scalars (`|` / `|-`).

**Marker search:**

When searching for a marker, we **ignore differences in indentation, trailing spaces, and the number of blank lines**, so that patches do not break because of formatting differences between various IDEs (Integrated Development Environments) and web viewers, including those used by neural networks when they fetch code from internet links.

We also **ignore comments in the programming language specified in `language`**. This is because a neural network often drops comments when rewriting the marker text. If the model does include a comment into the marker (which also happens), the tool will still parse this case correctly.

Marker processing rules:

* spaces and tab characters at the beginning and end of each line are removed;
* blank lines are ignored;
* the resulting `[begin, end]` range is computed over the real file lines, including the blank lines that were skipped for matching purposes.

From CHUNK’s point of view, the following markers are identical:

```text
         A

B
```

```text
A
            B
```

because they are both reduced to the canonical form

```text
A
B
```

---

## 3. `payload` Indentation

By default, the indentation of the inserted text (`payload`) is aligned with the indentation of the beginning of the marker, but this behavior can be changed:

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

Indentation modes:

* `from-marker` / `marker` / `auto` — align the `payload` to the marker’s indentation
  (this is the default mode if `options.indent` is not provided);
* `none` / `as-is` — leave `payload` indentation “as is” according to YAML rules.

The base indentation is taken from:

* for text commands — the first line of the marker range;
* for C++ / Python commands — the line where the class or method is declared.

Indentation is added only to **non-empty** lines of the `payload`.

---

## 4. File Commands

### 4.1. `create_file`

Create or overwrite a file:

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

* `path` — the target file.
* `payload` — file contents (may be empty → empty file).

### 4.2. `delete_file`

Delete a file:

```yaml
operations:
  - path: src/obsolete.cpp
    op: delete_file
```

---

## 5. Text Commands

These work with arbitrary text using a marker.

Required fields for a text operation:

* `path`
* `op: insert_text_after | insert_text_before | replace_text | delete_text | prepend_text | append_text`
* `marker` — required for marker-based operations (`insert_text_after`, `insert_text_before`, `replace_text`, `delete_text`).

For `prepend_text` and `append_text` the `marker` field is not used: they simply insert `payload` at the beginning or end of the file.

Additional fields:

* `before`, `after` — optional context (what surrounds the marker);
* `payload` — required for all except `delete_text`;
* `comment` — optional human-readable note for the operation;
* `options.indent` — optional.

### 5.1. `replace_text`

Replace a fragment by marker:

```yaml
operations:
  - path: src/foo.cpp
    op: replace_text
    marker: |-
      int value() const;
    payload: |-
      int value() const noexcept;
```

### 5.2. `insert_text_after`

Insert text immediately **after** the marker:

```yaml
operations:
  - path: src/foo.cpp
    op: insert_text_after
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

### 5.3. `insert_text_before`

Insert text **before** the marker:

```yaml
operations:
  - path: src/foo.cpp
    op: insert_text_before
    marker: |-
      public:
          void run();
    payload: |-
      public:
          void debug_run();
```

### 5.4. `delete_text`

Delete a fragment by marker:

```yaml
operations:
  - path: src/foo.cpp
    op: delete_text
    marker: |-
      #include "debug.h"
```

### 5.5. `prepend_text`

Insert text at the beginning of a file without specifying a marker:

```yaml
operations:
  - path: src/foo.cpp
    op: prepend_text
    payload: |-
      // This block will be inserted at the very beginning of the file.
      // ...
```

### 5.6. `append_text`

Insert text at the end of a file without specifying a marker:

```yaml
operations:
  - path: src/foo.cpp
    op: append_text
    payload: |-
      // This block will be added to the very end of the file.
      // ...
```

---

## 6. Commands for C++ / Python Symbols

These work with classes and methods rather than arbitrary text.

Supported commands:

* `replace_cpp_class`
* `replace_cpp_method`
* `replace_py_class`
* `replace_py_method`

Common fields:

* `path`
* `op`
* `payload` — new code of the class or method;
* `comment` — optional human-readable note for the operation;
* `options.indent` — indentation mode.

The target can be specified either:

* via `class` + `method`, or
* via `symbol`.

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

or the short form:

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

or:

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

## 7. Example of a Complete Patch

```yaml
description: "Update the value implementation in C++ and Python"
language: python

operations:
  - path: src/foo.cpp
    op: replace_cpp_method
    comment: "Update Foo::value implementation in C++"
    class: Foo
    method: value
    payload: |-
      int Foo::value() const
      {
          return 42;
      }

  - path: src/foo.py
    op: replace_py_method
    comment: "Update Foo.run implementation in Python"
    class: Foo
    method: run
    payload: |-
      def run(self):
          print("value:", self.value)
          return self.value
```
