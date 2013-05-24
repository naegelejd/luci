Luci
====

## Todo List

- Make object virtual methods interchangeable between types.
  The following two examples call two different internal functions,
  one being a _string_ method, the other an _int_ method:

        $ "hi" * 4

        $ 4 * "hi"

- Track symbol names throughout compilation/runtime.
  This would be useful when printing bytecode, as well as for error messages
- Update ConstantTable to actually de-duplicate constant Luci type objects
- Expand `map` type to allow for non-string keys
- Develop a module/import system. Put C-math functions in a math module.
- Add exception framework (setjmp/longjmp)
- Write standalone scanner/parser. This is counterintuitive considering the
  reliability and quality of *flex* and *bison*, however it will remove
  two build dependencies and provide a productive learning experience.
- Serialize bytecode/symbols/constants (like Python's .pyc files)
- Finalize syntax (i.e. make it awesome, maybe remove semicolons)
- Possibly hash all C-strings created during runtime, allowing for a single
  instance of each allocated Luci string
- Improve bytecode compiler
  - Append instructions faster (currently function call for each)
  - Implement short-circuit evaluation of conditional expressions
  - Bytecode optimizations
- Provide file manipulation functions (e.g. iteration through lines in a file)
- Provide string manipulation functions
- Implement Luci strings as unicode strings
- Finalize API for creating libraries in C
- Expand on interactive mode (don't require Ctrl+D(EOF))
- Update all `delete`/`free` functions to take a double-pointer so that
  the value of the pointer can be set to NULL (defensive programming)

        static void XXX_delete(XXX **xxx)
        {
            free(*xxx);
            *xxx = NULL;
        }

### Version 0.3

The following features have been implemented for Luci v0.3:

- Mark-and-Sweep garbage collection (with object finalization)
- Support for both empty strings and escape characters

### Version 0.2

The following features have been implemented for Luci v0.2:

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
