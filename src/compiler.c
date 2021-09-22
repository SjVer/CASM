#include "common.h"
#define INVALID_CONSUME -1

Instruction newInstruction(int opcode, const char *name)
{
	Instruction instr;
	instr.opcode = opcode;
	instr.name = cpystr(name, strlen(name));
	instr.args = newArray(0, 1, true);
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

// =============================

typedef void* (*consumeFn)(const char *part, const char *name, bool inList);
char *curLine;
int lineNo;

static void *consumeInt(const char *part, const char *name, bool inList)
{
	if (!isnum(strpstr(part, " "), false))
	{
		errorAt(curLine, lineNo, fstr("option '%s' expects %s, not '%s'.", 
			name, inList ? "integers" : "an integer", part));
		return (void *)INVALID_CONSUME;
	}
	return ADDR(int, atoi(part));
}

static void *consumeStr(const char *part, const char *name, bool inList)
{
	char *str = strpstr(part, " ");
	if (!strstart(str, "\"") && !strend(str, "\""))
	{
		errorAt(curLine, lineNo,
			fstr("option '%s' expects %s enclosed by double quotes, not '%s'.", 
				name, inList ? "strings" : "a string", str));
		return (void *)INVALID_CONSUME;
	}
	return cpystr(str + 1, strlen(str) - 2);
}

static Array consumeList(const char *part, const char *name, consumeFn fn)
{
	Array arr = _newArray(0, 1, true);
	Array parts = spltstr(part, ",");

	for (int i = 0; i < parts.used; i++)
	{
		char *str = strpstr(idxArray(&parts, i, char*), " ");
		
		/*
		create a new void pointer cuz otherwise the addresses of all items 
	   	will be the same and that's shit n stuff. The order of operations
	   	here is not to be meddled with either.
	   	*/
		
		void *ptr = malloc(sizeof(void*));
		void *item = fn(str, name, true);

		if (item == (void *)INVALID_CONSUME) return arr;
		
		memcpy(ptr, item, sizeof(void*));
		// printf(" ~ %d => %p\n", *(int *)ptr, ptr);
		_appendArray(&arr, ptr);
	}
	return arr;
}

// =============================

static AssembleStatus compileInstrFile(Options *options, const char *src)
{
	// split into lines
	Array lines = spltstr(src, "\n");
	// for(int i = 0; i < lines.used; i++)
	// 	printf("%s\n", idxArray(&lines, i, char*));

	for(lineNo = 1; lineNo < lines.used + 1; lineNo++)
	{
		// lineNo = lineNo;
		curLine = idxArray(&lines, lineNo - 1, char*);
		
		// option
		if (strstart(curLine, "#"))
		{
			// #bits {int}
			if (strstart(curLine + 1, "bits"))
			{	
				options->bitwidth = *(int *)consumeInt(curLine + 5, "bits", false);
				if (options->bitwidth == INVALID_CONSUME)
					return ASSEMBLE_INVALID_OPTION;

				msgAt(lineNo, fstr("Bitwidth set to %d bits.", options->bitwidth));
			}

			// #type {"text"|"bin"}
			else if (strstart(curLine + 1, "type"))
			{
				char *arg = strpstr(curLine + 5, " ");

				if (strcmp(arg, "text") == 0)
					options->outputType = TYPE_TEXT;
				else if (strcmp(arg, "bin") == 0)
					options->outputType = TYPE_BINARY;
				
				else
				{
					errorAt(curLine, lineNo, 
						fstr("option 'type' expects 'text' or 'bin', not '%s'.", arg));
					return ASSEMBLE_INVALID_OPTION;
				}

				msgAt(lineNo, fstr("Output type set to %s.", arg));
			}	

			// #ext {string}
			else if (strstart(curLine + 1, "ext"))
			{
				char *ext = consumeStr(curLine + 4, "ext", false);
				if (ext == (char *)INVALID_CONSUME) return ASSEMBLE_INVALID_OPTION;

				options->extension = ext;
				msgAt(lineNo, fstr("Extension set to '%s'.", options->extension));
			}

			// #prefix {string|ints}
			else if (strstart(curLine + 1, "prefix"))
			{
				char *arg = strpstr(curLine + 7, " ");

				if (options->outputType == TYPE_BINARY)
				{
					Array args = consumeList(arg, "prefix", &consumeInt);
					
					int *prefix = malloc(args.used * sizeof(int));
					for (int i = 0; i < args.used; i++)
						prefix[i] = idxArray(args, i, int);

					options->prefix = prefix;

					char *msg = "Prefix set to '";
					for (int i = 0; i < args.used; i++)
						msg = fstr("%s%s0x%x'", msg, i == 0 ? "" : "\b ", prefix[i]);

					msgAt(lineNo, msg);
				}
				else if (options->outputType == TYPE_TEXT)
				{
					char *prefix = consumeStr(arg, "prefix", false);
					if (prefix == (char *)INVALID_CONSUME) return ASSEMBLE_INVALID_OPTION;
					
					options->prefix = prefix;
					msgAt(lineNo, fstr("Prefix set to '%s'.", options->prefix));
				}
			}

			// #suffix {string|ints}
			else if (strstart(curLine + 1, "suffix"))
			{
				char *arg = strpstr(curLine + 7, " ");

				if (options->outputType == TYPE_BINARY)
				{
					Array args = consumeList(arg, "suffix", &consumeInt);
					
					int *suffix = malloc(args.used * sizeof(int));
					for (int i = 0; i < args.used; i++)
						suffix[i] = idxArray(args, i, int);

					options->suffix = suffix;

					char *msg = "Suffix set to '";
					for (int i = 0; i < args.used; i++)
						msg = fstr("%s%s0x%x'", msg, i == 0 ? "" : "\b ", suffix[i]);

					msgAt(lineNo, msg);
				}
				else if (options->outputType == TYPE_TEXT)
				{
					char *suffix = consumeStr(arg, "suffix", false);
					if (suffix == (char *)INVALID_CONSUME) return ASSEMBLE_INVALID_OPTION;
					
					options->suffix = suffix;
					msgAt(lineNo, fstr("Suffix set to '%s'.", options->prefix));
				}
			}

			// #format {string|ints}
			else if (strstart(curLine + 1, "format"))
			{
				char *arg = strpstr(curLine + 7, " ");

				if (options->outputType == TYPE_BINARY)
				{
					errorAt(curLine, lineNo, "format in binary output mode not supported.");
					return ASSEMBLE_INVALID_OPTION;
				}
				else if (options->outputType == TYPE_TEXT)
				{
					char *format = consumeStr(arg, "format", false);
					if (format == (char *)INVALID_CONSUME) return ASSEMBLE_INVALID_OPTION;
					
					options->format = format;
					msgAt(lineNo, fstr("Format set to '%s'.", options->format));
				}
			}

			// invalid option
			else
			{
				errorAt(curLine, lineNo, fstr("invalid option '%s'.",
					idxArray(spltstr(curLine + 1, " "), 0, char *)));
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