# chunk

`chunk` is a small CLI (command-line interface) tool that applies **structured patches** to a source tree.

Patches are written in the **chunk v2** YAML (YAML Ain't Markup Language) format, described in [`CHUNK.md`](https://mirmik.github.io/chunk/CHUNK.html).
All operations are applied **transactionally**: on any error, changes are rolled back.

---
## Features

- Applies patches in chunk v2 YAML format
- File operations: create and delete files
- Text operations using robust markers (tolerant to whitespace changes)
- C++ and Python symbol-level operations (replace class or method)
- Transactional apply with automatic backup and rollback

See [`CHUNK.md`](https://mirmik.github.io/chunk/CHUNK.html) or run:

```bash
chunk --help
```

for format details and examples.

---
## Usage

Basic usage:

```bash
chunk path/to/patch.yml
```

You can also read a patch from standard input:

```bash
generate_patch | chunk --stdin
wl-paste | chunk --stdin
```

Or from the system clipboard:

```bash
chunk --paste
```

On Linux and macOS, `chunk` will try (in this order): `wl-paste`,
`xclip -selection clipboard -o`, `xsel -b`, `pbpaste`.

On Windows it uses:

```powershell
powershell -command Get-Clipboard
```

Notes:

* Paths in the patch are relative to the current working directory.
* If a marker, class, or method cannot be found (or is ambiguous), the patch is aborted and all affected files are restored.

---
## Install Linux

Requirements:

* CMake ≥ 3.14
* C++17 compiler (for example `g++` or `clang++`)

Build and install:

```bash
git clone https://github.com/mirmik/chunk.git
cd chunk
mkdir build && cd build

cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
cmake --install .
```

By default the binary is installed into `${CMAKE_INSTALL_PREFIX}/bin`
(usually `/usr/local/bin` on most distributions).

---

## Install Windows
### Visual Studio

Requirements:

* Visual Studio 2019 or newer with C++ support
* CMake ≥ 3.14

Build and install:

```powershell
git clone https://github.com/mirmik/chunk.git
cd chunk

cmake -S . -B build -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
cmake --install build --config Release
```

The installer will place `chunk.exe` into `${CMAKE_INSTALL_PREFIX}\bin`
(unless you override `CMAKE_INSTALL_PREFIX`).

### MinGW / MSYS2

In a MinGW MSYS2 shell:

```bash
git clone https://github.com/mirmik/chunk.git
cd chunk
mkdir build && cd build

cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build .
cmake --install .
```

---

## Tests

After configuring the build:

```bash
cmake --build . --target chunk_tests
./chunk_tests
```

---

## License

Copyright © 2025 mirmik

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.