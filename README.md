Luci                                         
====

Luci is a procedural, dynamically-typed toy scripting language, implemented in C.

### Syntax

Luci's syntax resembles that of the languages C and Python, however
the syntax is still a work in progress.

### Tools Needed to Build

- [flex (lex)](http://flex.sourceforge.net/)
- [bison (yacc)](http://www.gnu.org/software/bison/)
- [doxygen 1.8.3](http://www.doxygen.org) to generate documentation
- a C compiler (clang, gcc)

### Implementation

Luci code is parsed to form an abstract syntax tree, which is then compiled
to bytecode and executed by a virtual machine.

### TODO

- Finish developing garbage collection framework
  (`free` out-of-scope Luci Objects throughout runtime)
  This will theoretically eliminate the need to store
  reference counts for each Luci type object!
- Track symbol names throughout compilation (useful when printing bytecode)
- Expand `map` type to allow for non-string keys
- Update ConstantTable to actually de-duplicate constant Luci type objects
- Develop a module/import system. Put C-math functions in a math module.
- Write standalone scanner/parser. This is counterintuitive considering the
  reliability and quality of *flex* and *bison*, however it will remove
  two build dependencies and provide a productive learning experience.
- Serialize bytecode/symbols/constants (like Python's .pyc files)
- Finalize syntax (i.e. make it awesome, maybe remove semicolons)
- Possibly hash all C-strings created during runtime, allowing for a single
  instance of each allocated Luci string
- Update all `delete`/`free` functions to take a double-pointer so that
  the value of the pointer can be set to NULL (defensive programming)

      static void XXX_delete(XXX **xxx)
      {
          free(*xxx);
          *xxx = NULL;
      }

- Improve bytecode compiler
  - Append instructions faster (currently function call for each)
  - Implement short-circuit evaluation of conditional expressions
  - Bytecode optimizations
- Provide file manipulation functions (like iteration through lines in a file)
- Provide string manipulation functions
- Add support for both empty strings and escape characters
- Implement Luci strings as unicode strings
- Finalize API for creating libraries in C
- Expand on interactive mode (don't require Ctrl+D(EOF))

### Version 0.2

The following features have been completed for Luci v0.2.

- Bytecode compiler
  - Instruction set is a work in progress
  - Cleanly store loop state for backpatching `BREAK` and `CONTINUE`
  - Efficient symbol table
- Bytecode interpreter (VM)
  - Stack-based
  - Fast instruction dispatch (gcc *computed goto* vs. *switch-case*)
  - Infinite function call stack
- Crude interactive mode (similar to Python's `>>>`)
- `map` type (similar to Python's `dict`)
  - Uses quadratic and double hashing 
- Virtual method tables for each built-in object type
  object types.

### Version 0.1

The following features were completed for Luci v0.1.
Luci v0.1 simply walked the abstract syntax tree and,
for each node, executed analogous expressions in C, i.e.
for each *if-else* node, a C function in the AST walking
code would recursively evaluate the conditional expression,
then if it was `true`, evaluate the *if-statements* node,
otherwise it would evaluate the *else-statements* node.

This version is now obsolete, as the implementation
was inefficient and did not allow for nested control flow
statements, i.e. `break`, `continue`, `return`.

- Implement all unary/binary operations offered by the C++ standard
  (with proper operator precedence for each)
- Implement Integer, Double, and String types
- Implement a While loop construct
- Implement error recover (made a single exit point `die()`), which has global
  access to the root ASTNode and root ExecEnviron
- Implement if/else conditional blocks. Decided against 'else if', which simplifies parser.
- Implement multi-parameter functions
- Implement a FILE * luci type, with open(), close(), read(), write() functions
- Create list types (syntax/parse), rename 'parameter' AST nodes since they're lists
- Implement For loops
- Re-work allocation/deallocation of LuciObjects to incorporate
  reference counts.
- Rewrite list implementation to use dynamic array of pointers (rather than singly-linked
  list. The singly-linked list was far less convenient since I'm using lists to implement
  function parameters.
- Implement user-defined functions (barely)
- Track line numbers in abstract syntax tree for more helpful Runtime Error messages.


### References

- [Immensely helpful](http://stackoverflow.com/a/2644949)
- [Just as useful](http://gnuu.org/2009/09/18/writing-your-own-toy-compiler/)
- [One of my questions](http://stackoverflow.com/q/13094001/1689220)
- [Implementation of Lua (PDF)](http://www.lua.org/doc/jucs05.pdf)
- [Interpreter implementation options](http://realityforge.org/code/virtual-machines/2011/05/19/interpreters.html)
- [Python's Innards](http://tech.blog.aknin.name/2010/04/02/pythons-innards-introduction/)
- [Bit Twiddling Hacks](http://graphics.stanford.edu/~seander/bithacks.html)
