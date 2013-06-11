Luci
====

Luci is a procedural, dynamically-typed toy scripting language, implemented in C.

Luci's syntax is similar to that of C and Lua.

The Luci command-line interpreter includes a crude interactive mode, which allows for quick exploration of the language and its runtime facilities.

Luci is missing an incredible number of features, many of which are listed under [TODO](TODO.md).

Luci's builtin types and functions are described in [LANGUAGE](LANG.md).

Sample programs written in Luci are provided in the `samples/` directory.

### Build 

To generate the necessary scanner and parser, you'll need
[flex (lex)](http://flex.sourceforge.net/) and
[bison (yacc)](http://www.gnu.org/software/bison/)

Both `flex` and `bison` are available via:

- any Linux distribution's package manager
- [Macports](http://www.macports.org/) on OSX
- [GnuWin32](http://gnuwin32.sourceforget.net/) on Windows.

You'll also need [cmake](http://www.cmake.org/) to generate the build system for your OS (Makefiles, VS Studio project, etc.)

Unix/MinGW build example:

    mkdir build
    cd build
    cmake ..
    make
    make test

This generates the binary `luci` in `build/bin`.

The included tests can be executed using `make test`, which utilizes Cmake's CTest framework.

To generate the included source documentation, obtain [doxygen 1.8.3](http://www.doxygen.org), then run `make doc`.

### Implementation

Luci code is parsed to form an abstract syntax tree, which is then compiled to Luci's bytecode representation, then executed by the virtual machine. 

All types in Luci are implemented using a base *LuciObject* `struct`, which contains a pointer to a virtual method table.
Each type implements its own static virtual method table.

*LuciObject* s are prevalent throughout Luci's implementation.
The *LuciList* is probably the most widely used, as it is essentially a dynamically expandable list of pointers.
Luci's virtual machine uses a *LuciList* for its internal stack.

Luci uses a crude, stop-the-world, mark-and-sweep garbage collector to allocate and manage memory.
This allows for simpler implementation code (less memory management) as well as the ability to *finalize* objects.

### References

- [Immensely helpful](http://stackoverflow.com/a/2644949)
- [Just as useful](http://gnuu.org/2009/09/18/writing-your-own-toy-compiler/)
- [One of my questions](http://stackoverflow.com/q/13094001/1689220)
- [Implementation of Lua (PDF)](http://www.lua.org/doc/jucs05.pdf)
- [Interpreter implementation options](http://realityforge.org/code/virtual-machines/2011/05/19/interpreters.html)
- [Python's Innards](http://tech.blog.aknin.name/2010/04/02/pythons-innards-introduction/)
- [Bit Twiddling Hacks](http://graphics.stanford.edu/~seander/bithacks.html)
