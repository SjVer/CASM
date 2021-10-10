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

#define ADDR(type, literal) &(type){literal}

// =========== array stuff ===========
typedef struct
{
    size_t used, size; //, typeSize;
    int growConstant;
    bool addNotMultiplySize;
    void **items;
} Array;

Array newArray(
    /*size_t typeSize,*/ int initCount,
    int growConstant, bool addNotMultiplySize);
void _appendArray(Array *array, void *item);

// #define newArray(/*type,*/ initCount, growConstant, addNotMultiplySize) \
//     (_newArray(/*sizeof(type),*/ initCount, growConstant, addNotMultiplySize))
#define appendArray(array, type, item) \
    (_appendArray(array, &(type){item}))

#define appendArrayCpy(array, item) \
    (_appendArray(array, memcpy(malloc(sizeof(item)), &item, sizeof(item))))

// #define appendArrayCpy(array,item) do { \
  void *new = malloc(sizeof(item)); \
  memcpy(new,&(item),sizeof(item)); \
  _appendArray(array,new); \
} while (0)

#define idxArray(array, index, type) \
    (*(type *)array.items[index])

// =========== ========= ===========

void *reallocate(void *pointer, size_t newSize);
void *newptr(void *src, size_t size);

char *cpystr(const char *chars, int length);
Array spltstr(const char *str, const char *delim);
char *strpstr(const char *str, const char *delim);
char *_strpstr(const char *str, const char *delim,
               bool front, bool back);
char *strpstrf(const char *str, const char *delim);
char *strpstrb(const char *str, const char *delim);
char *escstr(const char *str);
char *fstr(const char *format, ...);
bool isnum(const char *str, bool float_allowed);
bool strstart(const char *str, const char* start);
bool strend(const char *str, const char *end);
size_t utf8len(char *s);
char *bitsf(int value, int len);
char *toUpper(const char *str);

int bitlen(uint32_t value);
char *readFile(const char *path);
Array readFileLines(const char *path);
int panToUint8Arr(int *values, int count, int bitwidth, uint8_t **buffer);

#endif