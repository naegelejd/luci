#ifndef DRIVER_H
#define DRIVER_H

void *alloc(size_t size);
void yak(const char *, ... );
void die(const char *, ... );
void cleanup(void);


#endif
