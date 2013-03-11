Compilation Order
==================

#. All function names (add to global symbol table)
#. All global statements
#. All statements inside of functions.


Compilation Structure
=======================

- Stores instruction array
- Stores allocated symbol tables

  - "locals"
  - "globals"
  - "constants"

- Stores loop references for breaks/continues


On Compilation of Function
===========================

- Allocate new compiler structure
- Create new symbol table for "locals"
- "globals" symbol table equals current compiler structure's "locals"
- Insert 'empty' symbol for each function parameter
- ``compile()`` the function's statements using the new compiler structure
- add an empty RETURN instruction at the end if missing (guarantee a return)
- Add "function" object containing pointer to this structure to global symbol table


On Interpretation of *CALL*
============================

#. Pop function object
#. Save instruction pointer in current frame
#. Push current frame onto call stack
#. Replace current frame with frame pointed to by function object
#. Set instruction pointer to zero
#. Populate first **n** symbols with function arguments
#. Continue interpreting

On Interpretation of *RETURN*
==============================

#. Delete current frame ?
#. Pop frame off top of call stack, replacing current frame
#. Restore instruction pointer from the saved frame
#. Continue interpreting
