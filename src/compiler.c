#include "common.h"

Instruction newInstruction(int opcode)
{
	Instruction instr;
	instr.opcode = opcode;
	instr.argc = 0;
	instr.args = newArray(sizeof(Arg), 0, 1, true);
	return instr;
}