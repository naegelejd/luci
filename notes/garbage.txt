The garbage collector maintains a single heap (provided by malloc).

EVERYTHING are allocated via the garbage collector.
The size of each allocated object is irrelevant.

    def collect(roots):
        for each root in roots:
            gc(root)

    def gc(obj):
        get a list of pointers that obj references and the size of each child referenced
        for each pointer and size:
            copy pointer:pointer+size
            put forwarding pointer at child's former address
            update obj's reference to new child's address
            gc(child)
    

Native function support
An issue arises if, in a native C function, the following occurs:
 1. allocate container object
 2. begin allocating objects to put in container
 3. garbage collection runs while allocating a child object
The container object is not yet referenced by any roots (e.g. symbols or interpreter stack)
so the default collection algorithm will collect the container and children. The container
object is no longer valid, but the native function will likely continue appending/inserting
child objects.

Solution:
Require native functions to supply the address of the local pointer being used to store
the result of the allocation:

    LuciList *list = gc_alloc(LuciListType, &list);

The collector will keep track of these pointers and simply update the local pointer remotely
if collection occurs mid-function. The native function need never be aware that it's local
pointer to the container object changed where it's pointing to altogether.



OLD NOTES
-------------------------------------------------------------------------------
The garbage collector contains an arena structure for each possible size of
LuciObject. Each arena contains a dynamic array of pool structures, and a count
of how many pools are allocated and active. Each pool contains a static byte
array of size 4032, and a corresponding bitmask capable of flagging each slot
in the pool as used or available. A pool size of 4032 bytes allows a variable
number of LuciObjects to fit evenly in each pool (the size of a LuciObject is
always a multiple of 8/4).

The garbage collector implements a malloc-equivalent `gc_malloc`, which searches for available slots in available pools. `gc_free` is unnecessary as objects are automatically freed when garbage is collected.

Garbage is collected when all available pools are filled. If no garbage can be collected, the number of pools in the arena is doubled. This prevents garbage collection from occurring more often than necessary.

In order to collect garbage, the collector must be aware of which objects are currently in scope.

Potential garbage:

- all objects created in a function that has already returned.
- closed file objects
- objects that were once bound to names, or members of a container,
  but have since been 'overwritten'
  (e.g. the name is bound to a new object)

Gotchas:

- a function creates some objects then subsequently calls
  another function or itself.

Solution:

- Iterate through the frame stack, verifying that each object in
  each frame's constants, locals, and globals arrays are not collected.
- Iterate through object stack, ensuring nothing on stack is collected.
  This will prevent freeing things like iterators, which are not bound
  to names.
- Everything else is collected?...


Design:

- arena for each LuciObject size
- each arena contains a dynamic array of pools, and a count of active/allocated
  pools
- pool of size 4032 bytes. objects of sizes 8,16,24,32,48,56,64 divide evenly
  into 4032 and it's only 64 bytes under 4K.
- each pool has an array of bitflags to mark slots as empty or used

a call to gc_alloc will search the arena corresponding to the request size
starting at the most recently allocated pool for two reasons:
  1. recently allocated memory is more likely to be recycled (TODO: defend this)
  2. recently allocated memory is more likely to be cached

garbage is collected when either:
  no pools contain an empty slot
    or
  the arena needs to be dynamically expanded for more pools

either way, when garbage is collected, an attempt will be made to allocate more
pools to avoid future garbage collection. this could potentially depend on
how many objects are actually collected (i.e. many objects collected means more pools
may not be needed any time soon).

gc_collect must call some collection of 'get object array' functions to obtain
the states of:
  - the current interpreter stack
  - all Frames on the frame stack
  - all local variables in each Frame
  - all global symbols
unfortunately, this will require the interpreter to operate on global object
and frame stacks, preventing luci from being used as a multithreaded library.

gc_collect will then create an collection (array/list/tree) of live LuciObjects
from the collections provided by the interpreter.

gc_collect will then iterate through every object in every pool in every arena,
deleting any LuciObject whose pointer is not in the collection of live objects.


what if:
garbage collector provides stacks for objects and functions so at collection
time it can easily iterate through them to determine the root set?
