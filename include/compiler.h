#ifndef COMPILER_H
#define COMPILER_H

#include "common.h"

// output format type
typedef enum
{
	TYPE_TEXT,
	TYPE_BINARY,
} OutputType;

// argument for instructions
typedef struct
{
	int bitwidth;
	char *name;
} Arg;

// instruction
typedef struct
{
	int opcode, argc;
	Array args;
} Instruction;

Instruction newInstruction(int opcode);

// assembler options
typedef struct
{
	int bitwidth;
	OutputType outputType;
	char *extension, prefix, suffix, format;
} Options;


#endif