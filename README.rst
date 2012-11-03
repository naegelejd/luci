******
Luci
******

Luci is a toy scripting language, implemented in C.

Luci's syntax is still a work in progress, but the overall
theme is a mix of Python, Ruby and Perl.

The implementation is slowly developing toward a bytecode-compiled,
virtual machine interpreter model, from its original model, which
walked an abstract syntax tree and 'executed' each node.

Binaries
=========
Coming soon... Linux/Mac first, followed by Windows.

Tools Needed to Build
=======================
- `flex (lex)`_
- `bison (yacc)`_
- a decent C compiler (gcc)

.. _flex (lex): http://flex.sourceforge.net/
.. _bison (yacc): http://www.gnu.org/software/bison/

References
============
- `Immensely helpful`_
- `Just as useful`_

.. _Immensely helpful: http://stackoverflow.com/a/2644949
.. _Just as useful: http://gnuu.org/2009/09/18/writing-your-own-toy-compiler/

TODO (version 0.2)
===================

- Finalize syntax
- Implement bytecode compiler

  - Determine/iteratively improve instruction set
  - Implement efficient symbol table (hashing)
  - Implement useful constant table
  - Design function prototypes

- Implement bytecode interpreter (VM)

  - Efficient object stack
  - Fast instruction dispatch (gcc ``computed goto`` vs. ``switch-case``)
  - Proper function call/return handling

- Possibly implement an API for creating libraries

Completed in version 0.1
=========================
The following features were completed in Luci v0.1.
This version is nearly obsolete, as the implementation
was inefficient and did not allow for nested control flow
statements, i.e. ``break``, ``continue``, ``return``.

#. Implement all unary/binary operations offered by the C++ standard
   (with proper operator precedence for each)
#. Implement Integer, Double, and String types
#. Implement a While loop construct
#. Implement error recover (made a single exit point `die()`), which has global
   access to the root ASTNode and root ExecEnviron
#. Implement if/else conditional blocks. Decided against 'else if', which simplifies parser.
#. Implement multi-parameter functions
#. Implement a FILE * luci type, with open(), close(), read(), write() functions
#. Create list types (syntax/parse), rename 'parameter' AST nodes since they're lists
#. Implement For loops
#. Re-work allocation/deallocation of LuciObjects to incorporate
   reference counts.
#. Rewrite list implementation to use dynamic array of pointers (rather than singly-linked
   list. The singly-linked list was far less convenient since I'm using lists to implement
   function parameters.
#. Implement user-defined functions (barely)
#. Track line numbers in abstract syntax tree for more helpful Runtime Error messages.

Syntax Ideas
=============

-  function definitions::

      identifier(param1, param2, etc.) {

      }

      identifier(arg1, arg2, etc)

-  conditional blocks::

      if (condition) {

      } else {

      }

-  list range::

      0..9, 0..42..3    (ruby)

   or::

      range(10), range(0, 42, 3)    (python)

-  some kind of block comments, not just single line ``#...`` comments

