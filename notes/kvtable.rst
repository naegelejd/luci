/*
Luci's Hash Table
==================
A general-purpose hash table for both compilation and run-time

Compilation:

1. the hash table is used to hash constants (strings, ints, floats), so that for
each unique instance of each type, there is only one copy allocated in memory.

2. the hash table is used to hash symbol names to ensure that symbols are
properly referenced by LOAD, STORE, etc. instructions

Runtime:

1. the hash table is used to hash primitive Luci types (LuciString, LuciInt, LuciFloat)
to implement the LuciMap which stores LuciObject key-value pairs. Obviously
store/lookup actions on LuciMaps rely on LuciMaps containing unique keys.

Issues:

1. Storing and comparing ints, floats, and strings after they've been hashed.
    Collisions will require comparison to determine if values have already been
    hashed and stored.

2. Deleting keys/values. Eached hashed key gets assigned an integer index
    into the 'objects' array. This index is incremented after each store.
    During compilation, symbols and constants are never deleted, however
    during runtime, a LuciMap may delete symbol. The index of the object
    deleted from the table should be reused in the future.


Requirements:

Compilation:

insert:
    pass a LuciObject
    return an index into its array

lookup:
    pass a LuciObject
    return index of object or -1

get:
    pass index
    return LuciObject

set:
    pass index


Runtime (Maps):

insert:
    pass a LuciObject

remove:
    pass a LuciObject
    return removed Object

lookup:
    pass a LuciObject
    return found LuciObject or NULL
