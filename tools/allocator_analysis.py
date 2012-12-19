import math
import numpy as np
import matplotlib.pyplot as plt

def bytecount(n, s):
    # size of a memory pool
    pool_size = 4096

    # the number of pools necessary to hold n objects
    num_pools = ((n * s) / pool_size) + 1

    # the # of bytes needed to flag each object in a pool
    flags_per_pool = (pool_size / s)

    # the # of pools rounded to the next power of 2
    # this is the size of the dynamic array of ALL pools
    #   which lives in the Arena struct. (multiply this by sizeof(pointer)
    arena_pools = (1 << (np.cast['uint32'](np.log2(num_pools)) + 1))

    # size of a pointer in C
    sizeof_ptr = 8

    # these are misc bytes for managing the Arena's dynamic pool-pointer array
    misc = 8

    return num_pools * (pool_size + flags_per_pool) + (arena_pools * sizeof_ptr) + misc


# Pretend we're allocating 10,000 individual objects at once
# e.g. a recursive fibonacci function
n = np.arange(10000)

# Longs, Floats, LibFunctions
s = 16
c = 'r'
#plt.plot(n, (8 + 4096) * (((n * s)/ 4096) + 1) + (4096 / s), c)
plt.plot(n, bytecount(n, s), c)
plt.plot(n, n * (s + 16), c + '--');

# Strings, Lists, FunctionFrames
s = 24
c = 'g'
#plt.plot(n, (8 + 4096) * (((n * s)/ 4096) + 1) + (4096 / s), c)
plt.plot(n, bytecount(n, s), c)
plt.plot(n, n * (s + 16), c + '--');

# Files and Maps
s = 32
c = 'b'
#plt.plot(n, (8 + 4096) * (((n * s)/ 4096) + 1) + (4096 / s), c)
plt.plot(n, bytecount(n, s), c)
plt.plot(n, n * (s + 16), c + '--');

plt.show()
