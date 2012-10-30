#ifndef CONSTANT_H
#define CONSTANT_H

#include "object.h"

typedef struct cotable
{
    int count;      /* Current # of objects in array */
    int size;       /* Total object array allocated size */
    LuciObject **objects;   /* Constant object array */
} ConstantTable;

ConstantTable *cotable_new();
void cotable_delete(ConstantTable *);
int constant_id(ConstantTable *, LuciObject *);
LuciObject *cotable_get(ConstantTable *, int);

#endif
