#ifndef COMPILER_H
#define COMPILER_H

#include "common.h"

// argument for instructions
typedef struct
{
	int bitwidth;
	char *name;
} Arg;

// instruction
typedef struct
{
	int opcode;
	char *name;
	Array args;
} Instruction;

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
Instruction newInstruction(int opcode, const char *name);
void initOptions(Options *options);
AssembleStatus assemble(Options *options, bool verbose,
	const char *instr_path, const char *asm_path,
	const char *instr_src, const char *asm_src);
#endif