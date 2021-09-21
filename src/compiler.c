#include "common.h"

Instruction newInstruction(int opcode, const char *name)
{
	Instruction instr;
	instr.opcode = opcode;
	instr.name = cpystr(name, strlen(name));
	instr.args = newArray(Arg, 0, 1, true);
	return instr;
}

void initOptions(Options *options)
{
	options->bitwidth = DEFAULT_BITWIDTH;
	options->extension = cpystr("", 0);
	options->format = cpystr("", 0);
	options->outputType = TYPE_BINARY;
	options->prefix = cpystr("", 0);
	options->suffix = cpystr("", 0);
}

// =============================

// displays an error with the given token and message
static void errorAt(const char *part, int line, const char *message)
{
	fprintf(stderr, "[line %d] Error", line);

	// if (token->type == TOKEN_EOF)
	// {
	// 	fprintf(stderr, " at end");
	// }
	// else
	{
		fprintf(stderr, " at '%s'", part);
	}

	fprintf(stderr, ": %s\n", message);
}

// displays the given message if verbose mode is on
static void msgAt(int line, const char *msg)
{
	// fprintf(stdout, "%sMsg: %s\n", print_line ? fstr("[line %d] ", line) : "", msg);
	fprintf(stdout, "[line %d] %s\n", line, msg);
}

static AssembleStatus compileInstrFile(Options *options, const char *src)
{
	// split into lines
	Array lines = spltstr(src, "\n");

	for(int i = 0; i < lines.used; i++)
	{
		char *line = idxArray(&lines, i, char*);
		
		// option
		if (strstart(line, "#"))
		{
			// #bits {int}
			if (strstart(line + 1, "bits"))
			{	
				char *bitsstr = strpstr(line + 5, " ");

				if (!isnum(bitsstr, false))
				{
					errorAt(line, i + 1, fstr("'%s' is not a valid integer.", bitsstr));
					return ASSEMBLE_INVALID_OPTION;
				}

				options->bitwidth = atoi(bitsstr);
				msgAt(i + 1, fstr("Bitwidth set to %d bits.", options->bitwidth));
			}

			// #type {"text"|"bin"}
			else if (strstart(line + 1, "type"))
			{
				char *arg = strpstr(line + 5, " ");

				if (strcmp(arg, "text") == 0)
					options->outputType = TYPE_TEXT;
				else if (strcmp(arg, "bin") == 0)
					options->outputType = TYPE_BINARY;
				
				else
				{
					errorAt(line, i + 1, 
						fstr("option 'type' expects 'text' or 'bin', not '%s'.", arg));
					return ASSEMBLE_INVALID_OPTION;
				}

				msgAt(i + 1, fstr("Output type set to %s.", arg));
			}	

			// #ext {string}
			else if (strstart(line + 1, "ext"))
			{
				char *arg = strpstr(line + 4, " ");
				
				if (!strstart(arg, "\"") && !strend(arg, "\""))
				{
					errorAt(line, i+1,
						fstr("option 'ext' expects a string enclosed by double quotes, not '%s'.", arg));
					return ASSEMBLE_INVALID_OPTION;
				}

				options->extension = cpystr(arg + 1, strlen(arg) - 2);
				msgAt(i + 1, fstr("Extension set to '%s'.", options->extension));
			}

			// #prefix {string|ints}
			else if (strstart(line + 1, "prefix"))
			{
				char *arg = strpstr(line + 7, " ");

				if (options->outputType == TYPE_BINARY)
				{
					errorAt(line, i + 1, "prefix in binary output mode not supported.");
					return ASSEMBLE_INVALID_OPTION;
				}
				else if (options->outputType == TYPE_TEXT)
				{
					if (!strstart(arg, "\"") && !strend(arg, "\""))
					{
						errorAt(line, i+1,
							fstr("option 'prefix' expects a string enclosed by double quotes, not '%s'.", arg));
						return ASSEMBLE_INVALID_OPTION;
					}
					options->prefix = cpystr(arg + 1, strlen(arg) - 2);
					msgAt(i + 1, fstr("Prefix set to '%s'.", options->prefix));
				}
			}

			// invalid option
			else
			{
				errorAt(line, i + 1, fstr("invalid option '%s'.",
					idxArray(&spltstr(line + 1, " "), 0, char *)));
				return ASSEMBLE_INVALID_MACRO;
			}
		}		
	}

	return ASSEMBLE_SUCCESS;
}

AssembleStatus assemble(Options *options, 
	const char *instr_src, const char *asm_src)
{
	AssembleStatus instrStatus = compileInstrFile(options, instr_src);
	return instrStatus;
}