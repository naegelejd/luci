Luci
====

## Types
Luci provides the following first-class types:

**int** - long integer

**float** - double-precision floating-point number

**string** - character arrays

**list** - lists of arbitrary Luci types

**maps** - hashtables with string keys and arbitrary values

**file** - OS-level files for reading/writing

**function** - both user-implemented or native functions

**nil** - singleton of no type

## Builtin Functions
Luci also provides the following builtin functions:

**print** - prints string representations of objects to stdout

**help** - prints a help string for a given object

**exit** - abruptly exits Luci

**input** - reads a line from stdin

**readline** - reads a line from a file

**typeof** - returns the type of a given object

**assert** - asserts that an expression is true

**copy** - returns a deep copy of a given object

**str** - casts an object to a string

**int** - casts an object to an int

**float** - casts an object to a float

**hex** - returns a string of the hex representation of an int

**fopen** - opens a file

**fclose** - closes a file

**fread** - reads a file

**fwrite** - writes a string to a file

**flines** - reads the lines in a file as a list

**range** - generates a range of integers

**sum** - computes the sum of a list of numbers

**len** - computes the length of a list

**max** - computes the sum of a list of numbers

**min** - computes the sum of a list of numbers

**contains** - determines if the given container contains a given object

## Builtin Values
As well as the following builtin values:

**stdout** - a Luci file corresponding to `stdout`

**stderr** - a Luci file corresponding to `stderr`

**stdin** - a Luci file corresponding to `stdin`

**e** - 2.718281828459045

**pi** - 3.141592653589793
