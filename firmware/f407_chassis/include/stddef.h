#ifndef PROJECT_STDDEF_H
#define PROJECT_STDDEF_H

typedef unsigned int size_t;
typedef int ptrdiff_t;

#ifndef NULL
#define NULL ((void *)0)
#endif

#define SIZE_MAX 4294967295UL
#define offsetof(type, member) ((size_t)&(((type *)0)->member))

#endif
