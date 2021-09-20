#ifndef TI_CHUNK_H
#define TI_CHUNK_H

#include "common.h"

// chunk stuff
typedef struct
{
    int count;
    int capacity;
    uint8_t *bytes;
    int *lines;
} Chunk;

void initChunk(Chunk *chunk);
void writeChunk(Chunk *chunk, uint8_t byte, int line);
void freeChunk(Chunk *chunk);

#endif