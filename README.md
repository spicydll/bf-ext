# bf-ext
A brainfuck interpreter with extended features, such as support for syscalls.

## Why?

Traditional Brainfuck is touted as "Turing complete," meaning that you can technically do anything in the language. However, in order to do things like reading files or executing other programs, you would need an OS built with some sort of brainfuck interop or some shit. Since aint nobody got time for that, I decided I would extend the language slightly to make it compatible with existing OS syscalls (only on 64 bit linux for now).

## What's different?

There are two new tokens that make calling syscalls possible.

### Syscall Operator: `$`

The `$` operator copies 48 bytes starting at the current pointer and sends them as arguments to the C stdlib `syscall()` function. It then replaces the next 8 bytes starting at the current pointer with the return value of the syscall.

### Dereference Operator: `@`

Since any pointer passed to a syscall needs an actual memory pointer and not an index in the array, we need a way to get the memory address of an index the programmer would know. When encountered, the interpreter takes the next 4 bytes starting at the current pointer and 
indexes into the memory array with it as if it were an integer. It then gets a pointer to that index in the memory array and stores it as 8 bytes starting at the current memory pointer.

## Build

Just compile in your favorite C compiler

```
gcc -o bf-ext bf-ext.c
```

## Run

Just supply the name of the bf file to interpret.

```
./bf-ext examples/hello.bf
Hello world!
```
