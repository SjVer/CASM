#ifndef COMPILER_H
#define COMPILER_H

#include "common.h"

// argument for instructions
typedef struct
{
	int bitwidth;
	int opcode;
	char *name;
} Arg;

// instruction
typedef struct
{
	char *name;
	Array args;
} Instruction;

// label
typedef struct
{
	int address;
	char *name;
} Label;

// output format type
typedef enum
{
	TYPE_TEXT,
	TYPE_BINARY,
} OutputType;

// assembler options
typedef struct
{
	int bitwidth;
	OutputType outputType;
	char *extension;
	void *prefix, *suffix, *format;
} Options;

// functions
Instruction newInstruction(Array Args_array, const char *name);
void initOptions(Options *options);
AssembleStatus assemble(Options *options, int verbose, Chunk *chunk,
	const char *instr_path, const char *asm_path,
	const char *instr_src, const char *asm_src);
#endif