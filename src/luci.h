/******************************************************************************
* Copyright (C) 2012 Joseph Naegele.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/

/**
 * @file luci.h
 */

#ifndef LUCI_LUCI_H
#define LUCI_LUCI_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>
#include <assert.h>

/** maximum allowed size of a raw string in a source file */
#define MAX_STR_CONST       8192
/** maximum # of digits needed to convert a long to a char* */
#define MAX_INT_DIGITS      (CHAR_BIT * (sizeof(long) / 3) + 3)
/** maximum # of digits needed to convert a double to a char* */
#define MAX_FLOAT_DIGITS    (CHAR_BIT * (sizeof(double) / 3) + 3)


/** an instruction is a 32-bit unsigned int */
typedef uint32_t Instruction;


#ifdef DEBUG

/**
 * Verbosely prints debug information
 *
 * @param fmt C-style formatting
 * @param ... everything to print
 */
#define LUCI_DEBUG(fmt, ...) \
    do { \
        fprintf(stderr, "%s:%d:%s(): " fmt, \
                __FILE__, __LINE__, __func__, __VA_ARGS__); \
    } while (0)

/**
 * Verbosely prints error information prior to exiting
 *
 * @param fmt C-style formatting
 * @param ... everything to print
 */
#define LUCI_DIE(fmt, ...) \
    do { \
        fprintf(stderr, "FATAL: %s:%d:%s(): " fmt, \
                __FILE__, __LINE__, __func__, __VA_ARGS__); \
        exit(1); \
    } while (0)

#else   /* DEBUG */

/**
 * Does not print any debug information.
 *
 * @param fmt C-style formatting
 * @param ... everything to not print
 */
#define LUCI_DEBUG(fmt, ...)

/**
 * Quietly prints error information prior to exiting
 *
 * @param fmt C-style formatting
 * @param ... everything to print
 */
#define LUCI_DIE(fmt, ...) \
    do { \
        fprintf(stderr, "FATAL: " fmt, __VA_ARGS__); \
        exit(1); \
    } while (0)

#endif  /* DEBUG */

/* Defined in gc.c */
void *alloc(size_t size);

#endif /* LUCI_LUCI_H */
