#ifndef TI_CHUNK_H
#define TI_CHUNK_H

#include "common.h"

// chunk stuff
typedef struct
{
    int count;
    int capacity;
    int *bytes;
    int *lines;
} Chunk;

void initChunk(Chunk *chunk);
void writeChunk(Chunk *chunk, int value, int line);
void freeChunk(Chunk *chunk);

#endif