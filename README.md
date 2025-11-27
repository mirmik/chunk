# chunk

`chunk` is a small CLI (command-line interface) tool that applies **structured patches** to a source tree.

Patches are written in the **chunk v2** YAML (YAML Ain't Markup Language) format, described in [`CHUNK.md`](CHUNK.md).  
All operations are applied **transactionally**: on any error, changes are rolled back.

---

## Features

- Applies patches in chunk v2 YAML format  
- File operations: create and delete files  
- Text operations using robust markers (tolerant to whitespace changes)  
- C++ and Python symbol-level operations (replace class or method)  
- Transactional apply with automatic backup and rollback  

See [`CHUNK.md`](CHUNK.md) or run:

```bash
chunk --help
````

for format details and examples.

---

## Usage

Basic usage:

```bash
chunk path/to/patch.yml
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

MIT
