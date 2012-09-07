luci
====

Luci is my toy programming language, implemented in C.

Like most of my side projects, a specific discipline in computer science
caught my eye and I started playing with it. I read about Lex/Yacc
and started with Bison's multi-function calculator tutorial. I then started
digging around online for examples of creating homemade abstract syntax trees.
Before I knew it I was working on a simple scripting language.

Unlike most of my side projects, I'm actually continuing to work on my
language implementation, even after reaching a point where I think I understand
most of the nuances of creating/interpreting a basic language.

The language resembles Python in many ways, mostly because Python's
syntax is very intuitive, and because Python is the best interpreted language.
I think my ultimate goal is to create a simple scripting language, capable
of performing basic tasks that I would normally do in Bash or simple Python.

My C is a few years rusty but I'll do my best to retroactively clean up messy stuff.

Binaries
---------
Coming soon... Linux/Mac first, then Windows.

Tools Needed to Build
-----------------------
- `flex (lex)`_
- `bison (yacc)`_
- a decent C compiler (gcc)

.. _flex (lex): http://flex.sourceforge.net/
.. _bison (yacc): http://www.gnu.org/software/bison/


References
------------
- `Immensely helpful`_
- `Just as useful`_

.. _Immensely helpful: http://stackoverflow.com/a/2644949
.. _Just as useful: http://gnuu.org/2009/09/18/writing-your-own-toy-compiler/

Additional
------------

- I use `graphviz`_ to view the bison graph output of my parser.

.. _graphviz: http://www.graphviz.org

TODO List
---------

#. Handle mixed-type binary operations (string * int)
#. Implement a FILE * luci type, with open(), close(), read(), write() functions
#. Implement For loops

Completed
---------

#. Implement all unary/binary operations offered by the C++ standard (with proper operator precedence for each)
#. Implement Integer, Double, and String types
#. Implement a While loop construct
#. Implement error recover (made a single exit point `die()`), which has global
   access to the root ASTNode and root ExecEnviron
#. Implement if/else conditional blocks. Decided against 'else if', which simplifies parser.
#. Implement multi-parameter functions

