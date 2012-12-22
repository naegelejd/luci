## Luci

Luci is a procedural, dynamically-typed toy scripting language, implemented in C.

Luci's syntax resembles C and is still a work in progress.

The implementation is slowly evolving into a bytecode-compiled,
virtual machine interpreter model. Its initial form consisted of
an abstract syntax tree and a series of functions which walked the
tree and 'executed' each node.


### Syntax

Luci's syntax heavily resembles that of the language C.


### Tools Needed to Build

- [flex (lex)](http://flex.sourceforge.net/)
- [bison (yacc)](http://www.gnu.org/software/bison/)
- a decent C compiler (gcc)


### References

- [Immensely helpful](http://stackoverflow.com/a/2644949)
- [Just as useful](http://gnuu.org/2009/09/18/writing-your-own-toy-compiler/)
- [One of my questions](http://stackoverflow.com/q/13094001/1689220)
- [Implementation of Lua (PDF)](http://www.lua.org/doc/jucs05.pdf)
- [Interpreter implementation options](http://realityforge.org/code/virtual-machines/2011/05/19/interpreters.html)
- [Python's Innards](http://tech.blog.aknin.name/2010/04/02/pythons-innards-introduction/)


### TODO (version 0.2)

- Add support for both empty strings and escape characters
- Track symbol names throughout compilation (useful when printing bytecode)
- Implement interactive mode (like Python's `>>>`)
- Serialize bytecode/symbols/constants (like Python's .pyc files)
- Finalize syntax
- Implement bytecode compiler

  - Determine/iteratively improve instruction set
  - Append instructions faster (currently function call for each)
  - Backpatch instructions faster (currently function call for each)
  - Cleanly store loop state for backpatching `BREAK` and `CONTINUE`
  - Implement efficient symbol table (hash)
  - Implement useful constant table (hash constants... currently storing duplicates)
  - Design function prototypes
  - Bytecode optimizations

- Implement bytecode interpreter (VM)

  - Efficient object stack
  - Fast instruction dispatch (gcc *computed goto* vs. *switch-case*)
  - Proper function call/return handling (call stack)

- Implement a memory manager optimized for allocating many small blocks.
  This heavily relies on a permanent design decision for Luci's
  types (objects) implementation (Fixed-size (union) or variable (Python)).

- Potentially integrate a garbage collector in the memory manager.
  This would eliminate the need to store reference counts for each
  object.

- Possibly implement an API for creating libraries


### Completed in version 0.1

The following features were completed in Luci v0.1.
Luci v0.1 simply walked the abstract syntax tree and,
for each node, executed analogous expressions in C, i.e.
for each *if-else* node, a C function in the AST walking
code would recursively evaluate the conditional expression,
then if it was `true`, evaluate the *if-statements* node,
otherwise it would evaluate the *else-statements* node.

This version is now obsolete, as the implementation
was inefficient and did not allow for nested control flow
statements, i.e. `break`, `continue`, `return`.

1. Implement all unary/binary operations offered by the C++ standard
   (with proper operator precedence for each)
2. Implement Integer, Double, and String types
3. Implement a While loop construct
4. Implement error recover (made a single exit point `die()`), which has global
   access to the root ASTNode and root ExecEnviron
5. Implement if/else conditional blocks. Decided against 'else if', which simplifies parser.
6. Implement multi-parameter functions
7. Implement a FILE * luci type, with open(), close(), read(), write() functions
8. Create list types (syntax/parse), rename 'parameter' AST nodes since they're lists
9. Implement For loops
10. Re-work allocation/deallocation of LuciObjects to incorporate
    reference counts.
11. Rewrite list implementation to use dynamic array of pointers (rather than singly-linked
    list. The singly-linked list was far less convenient since I'm using lists to implement
    function parameters.
12. Implement user-defined functions (barely)
13. Track line numbers in abstract syntax tree for more helpful Runtime Error messages.
