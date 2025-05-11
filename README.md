# Tyger

An experimental dynamically typed scripting lanugae, written as an extension to my
time reading and working through [Thorsten Ball's](https://thorstenball.com/)
[Writing An Interpreter In Go](https://interpreterbook.com/). Project interpreter
is implemented using _C_ (_C99_) as I wanted to improve some of my low level programming
ability and knowledge.

The language and interpreter are distributed under the terms of the [Apache 2.0](./LICENSE)
license.

---

# Samples

## Hello, World

```go
println("Hello, World!");
```

## Variables

```go
var x = 5 + 4 * 3 - 2 / 1;
println(x);
```

## Fibonacci Series

```go
const fib = func(n) {
    if (n < 2) {
        return 1;
    } else {
        return fib(n - 1) + fib(n - 2);
    }
};
```

---

# Building

## CMake

Run the following commands:

```sh
cmake -S . -B build
cmake --build build
```

Alternatively:

```sh
# 1. create the build dir & change to it
mkdir build
cd build
# 2. generate the build files
cmake -S .. -B .
# 3. build the project(s)
cmake --build .
```
