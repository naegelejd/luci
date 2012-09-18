
exec_func_def
---------------
Store the ASTNode in a symbol so the symbol knows

-  the # of params
-  the names of params
-  the statements
-  the return expression

Store the name of the func in the symbol

exec_call
----------
If name requested is a user defined function
give the symbol an ExecutionContext containing

-  a symbol table containing its own parameters
-  The parent ExecutionContext

get_symbol
-----------
Search primary ExecutionContext, then
global ExecutionContext if not found in primary

add_symbol
-----------
Only add symbols to primary ExecutionContext

