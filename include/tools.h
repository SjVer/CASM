#ifndef TI_TOOLS_H
#define TI_TOOLS_H

#include "common.h"

// allocates memory for type of size count
#define ALLOCATE(type, count) \
    (type *)reallocate(NULL, sizeof(type) * (count))

// frees smth from a pointer
#define FREE(type, pointer) reallocate(pointer, 0)

// duplicates capacity. (sets to 8 if capacity is 0)
#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity)*2)

// frees the memory of the array
#define FREE_ARRAY(type, pointer, oldCount) \
    reallocate(pointer, 0)

// grows array to new size using the type of data and the newCount
#define GROW_ARRAY(type, pointer, newCount) \
    (type *)reallocate(pointer, sizeof(type) * (newCount))

void *reallocate(void *pointer, size_t newSize);
char *fstr(const char *format, ...);
size_t utf8len(char *s);
char *readFile(const char *path);
void writeBinFile(const char *path, uint8_t *bytes, int byteCount);
char *toUpper(const char *str);

#endif