#include "common.h"

// disable "type name is not allowed" error
#ifdef __INTELLISENSE__
#pragma diag_suppress 254
#pragma diag_suppress 29
#endif

void initChunk(Chunk *chunk)
{
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->bytes = NULL;
    chunk->lines = NULL;
}

void writeChunk(Chunk *chunk, int value, int line)
{
    if (chunk->capacity < chunk->count + 1)
    {
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->bytes = GROW_ARRAY(
            int, chunk->bytes, chunk->capacity);
        chunk->lines = GROW_ARRAY(
            int, chunk->lines, chunk->capacity);
    }

    chunk->bytes[chunk->count] = value;
    chunk->lines[chunk->count] = line;
    chunk->count++;
}

void freeChunk(Chunk *chunk)
{
    FREE_ARRAY(int, chunk->bytes, chunk->capacity);
    FREE_ARRAY(int, chunk->lines, chunk->capacity);
    initChunk(chunk);
}
