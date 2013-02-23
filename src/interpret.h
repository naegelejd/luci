/*
 * See Copyright Notice in luci.h
 */

/**
 * @file interpret.h
 */

#ifndef INTERPRET_H
#define INTERPRET_H

#include "compile.h"

#define MAX_LIBFUNC_ARGS 256

void eval (Frame *);

#endif
