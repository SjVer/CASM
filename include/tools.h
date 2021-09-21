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

// =========== array stuff ===========
typedef struct
{
    size_t used, size, typeSize;
    int growConstant;
    bool addNotMultiplySize;
    void **items;
} Array;

Array _newArray(
    size_t typeSize, int initCount,
    int growConstant, bool addNotMultiplySize);
void _appendArray(Array *array, void *item);

#define newArray(type, initCount, growConstant, addNotMultiplySize) \
    (_newArray(sizeof(type), initCount, growConstant, addNotMultiplySize))
#define appendArray(array, item) (_appendArray(array, (void *)item))
#define idxArray(array, index, type) ((type)array.items[index])

/*
#define newArray(type, size) ({     \
    struct{                         \
        Array arr; type *items;     \
    } newArr;                       \
    newArr.items = malloc(          \
        size * sizeof(type));       \
                                    \
    newArr.arr.size = size;         \
    newArr.arr.used = 0;            \
    newArr;                         \
})
*/

// =========== ========= ===========

void *reallocate(void *pointer, size_t newSize);
char *fstr(const char *format, ...);
size_t utf8len(char *s);
char *readFile(const char *path);
void writeBinFile(const char *path, uint8_t *bytes, int byteCount);
char *toUpper(const char *str);

#endif