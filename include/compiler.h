#ifndef COMPILER_H
#define COMPILER_H

#include "common.h"

#define OUTPUT_PRINT_COUNT 8

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
	int argc;
	char *name;
	Array args;
} Instruction;

// label
typedef struct
{
	int address;
	char *name;
} Label;

// temp label
typedef struct
{
	char *name;
	// address = index in chunk
	// bitpos = index of bits of instruction
	//			where the value starts
	int address, bitpos, bitwidth, line;
} TmpLabel;

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
	int *prefix, prefixLen;
	int *suffix, suffixLen;
	int *format, formatLen;
	bool padLeading;
} Options;

// functions
// Instruction newInstruction(Array Args_array, const char *name);
void initOptions(Options *options);
AssembleStatus assemble(Options *options, int verbose, Chunk *chunk,
	const char *instr_path, const char *asm_path);
#endif