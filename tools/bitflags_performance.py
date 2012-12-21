"""
Proof that intensive operations on an array of 32-bit flags is more
efficient than on arrays of either 16-bit or 64-bit flags, in terms of
total instruction count to set each bit.

The examples consist of 3 large flags of varying size:

    1. 1024 bit flag
    2. 768 bit flag
    3. 512 bit flag

In each example, the flag composed of 32-bit unsigned integers
requires the least overall loop iterations.
"""

def setbit(flags, fmax):
    for i, flag in enumerate(flags):
        # only choose this flag if it has at least 1 zero bit
        if flag < fmax:
            r = 0
            # search for first zero bit starting at bit 0
            while flag & (1 << r):
                r += 1
            # set the bit
            flags[i] = flag | (1 << r)

            # return the total # of iterations performed (in any loop)
            return r + i
            # optionally: give weights to 'r' and 'i'
            # a heavier 'r' (r * 2) does better for small flags (16 bit)
            # while a heaver 'i' (i * 2) slighly speeds up large flags (64 bit)

    # all bits in all flags are set
    return -1


def count_iterations(numflags, flagmax):
    total = 0
    pools = 0
    while pools < ITERATIONS:
        flags = [0 for i in range(numflags)]
        while True:
            t = setbit(flags, flagmax)
            if t < 0:
                break
            total += t

        pools += 1

    return total


ITERATIONS = 20
print count_iterations(16, 0xFFFFFFFFFFFFFFFF)
print count_iterations(32, 0xFFFFFFFF)
print count_iterations(64, 0xFFFF)
print
print count_iterations(12, 0xFFFFFFFFFFFFFFFF)
print count_iterations(24, 0xFFFFFFFF)
print count_iterations(48, 0xFFFF)
print
print count_iterations(8, 0xFFFFFFFFFFFFFFFF)
print count_iterations(16, 0xFFFFFFFF)
print count_iterations(32, 0xFFFF)
