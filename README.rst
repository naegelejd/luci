luci
====

My toy language, implemented in C (for now).

Details/Instructions to come!

Tools Needed
---------------------
- `flex (lex)`_
- `bison (yacc)`_
- a decent C compiler (gcc)

.. _flex (lex): http://flex.sourceforge.net/
.. _bison (yacc): http://www.gnu.org/software/bison/


References
----------
- `Immensely helpful`_
- `Just as useful`_

.. _Immensely helpful: http://stackoverflow.com/a/2644949
.. _Just as useful: http://gnuu.org/2009/09/18/writing-your-own-toy-compiler/

Additional
----------

- I use `graphviz`_ to view the bison graph output of my parser.

.. _graphviz: http://www.graphviz.org

TODO List
---------

#. Handle mixed-type binary operations (string * int)
#. Implement multi-parameter functions
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

