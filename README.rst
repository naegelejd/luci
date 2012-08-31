luci
====

My toy language, implemented in C (for now).

Details/Instructions to come!

Tools Needed
---------------------
- `flex (lex)`_
- `bison (yacc)`_
- a decent C compiler (gcc)

.. _flex (lex): (http://flex.sourceforge.net/)
.. _bison (yacc): (http://www.gnu.org/software/bison/)


References
----------
- `Immensely helpful`_
- `Just as useful`_

.. _Immensely helpful: (http://stackoverflow.com/a/2644949)
.. _Just as useful: (http://gnuu.org/2009/09/18/writing-your-own-toy-compiler/)


TODO List
---------

# Implement error recovery (free all allocated memory before exit)
# Typedef all existing struct type enums and put in a types header?
# Change all instances of 'char op' to an 'enum op_type', in order to implement two-char ops(like ==, <=, >=, etc.)
# Add a double ASTNode type and LuciObject
# Implement a while loop
# Add a string ASTNode type and LuciObject
# Implement multi-parameter functions
# ?
