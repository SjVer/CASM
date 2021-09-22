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

char *curFile;
bool verbose;

// displays an error with the given token and message
static void errorAt(const char *part, int line, const char *message)
{
	fprintf(stderr, "[%s:%d] Error", curFile, line);

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
	if (verbose) fprintf(stdout, "[%s:%d] %s\n", curFile, line, msg);
}

// =============================

typedef void* (*consumeFn)(const char *part, const char *name, bool inList);
char *curLine;
int lineNo;

static void *consumeIntMacro(const char *part, const char *name, bool inList)
{
	if (!isnum(strpstr(part, " "), false))
	{
		errorAt(curLine, lineNo, fstr("%s expects %s, not '%s'.", 
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
			fstr("%s expects %s enclosed by double quotes, not '%s'.", 
				name, inList ? "strings" : "a string", str));
		return (void *)INVALID_CONSUME;
	}
	return cpystr(str + 1, strlen(str) - 2);
}

static Array consumeList(const char *part, const char *name, consumeFn fn)
{
	Array arr = newArray(0, 1, true);
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

static int consumeIntMaybe(const char *part)
{
	char *str = strpstr(strpstr(part, " "), "\t");

	printf("%s ", str);

	// normal int
	if (isnum(str, false)) return atoi(str);

	// hexadecimal
	else if (strstart(str, "0x"))
	{
		for (int i = 2; i < strlen(str); i++)
			if (!(tolower(str[i]) >= '0' && tolower(str[i]) <= 'f')) return INVALID_CONSUME;
		return (int)strtol(str + 2, NULL, 16);
	}

	// binary
	else if (strstart(str, "0b"))
	{
		for (int i = 2; i < strlen(str); i++)
			if (str[i] != '0' && str[i] != '1') return INVALID_CONSUME;
		return (int)strtol(str + 2, NULL, 2);
	}

	return INVALID_CONSUME;
}

// =============================

static AssembleStatus compileInstrFile(Options *options, const char *src)
{
	// split into lines
	Array lines = spltstr(src, "\n");

	for(lineNo = 1; lineNo < lines.used + 1; lineNo++)
	{
		curLine = idxArray(&spltstr(idxArray(&lines, lineNo - 1, char*), ";"), 0, char*);
		
		// option
		if (strstart(curLine, "#"))
		{
			// #bits {int}
			if (strstart(curLine + 1, "bits"))
			{	
				options->bitwidth = *(int *)consumeIntMacro(curLine + 5, "option 'bits'", false);
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
				char *ext = consumeStr(curLine + 4, "option 'ext'", false);
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
					Array args = consumeList(arg, "option 'prefix'", &consumeIntMacro);
					
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
					char *prefix = consumeStr(arg, "option 'prefix'", false);
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
					Array args = consumeList(arg, "option 'suffix'", &consumeIntMacro);
					
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
					char *suffix = consumeStr(arg, "option 'suffix'", false);
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
					char *format = consumeStr(arg, "option 'format'", false);
					if (format == (char *)INVALID_CONSUME) return ASSEMBLE_INVALID_OPTION;
					
					options->format = format;
					msgAt(lineNo, fstr("Format set to '%s'.", options->format));
				}
			}

			// invalid option
			else
			{
				errorAt(curLine, lineNo, fstr("invalid option '%s'.",
					idxArray(&spltstr(curLine + 1, " "), 0, char*)));
				return ASSEMBLE_INVALID_MACRO;
			}
		}		
	
		// instruction or macro
		else
		{
			Array words = spltstr(curLine, " ");
			Array args = newArray(0, 1, true);

			/*
			we only know if we're compiling a macro once
			we've finished the arguments so we first do that
			*/
			
			int i; // index of last word parsed
			bool macro = false;

			for (i = 1; i < words.used; i++)
			{
				char *curWord = strpstr(idxArray(&words, i, char*), "\t");

				if (!strstart(curWord, "{")) break; // no arg
				if (!(strend(curWord, "}") || strend(curWord, "}:")))
				{
					errorAt(curWord, lineNo, "Expected '}'.");
					return ASSEMBLE_INVALID_DECL;
				}
				
				_appendArray(&args, strcpy(malloc(strlen(curWord) + 1), curWord));

				if (strend(curWord, ":"))
				{
					// must be last word
					if (i + 1 != words.used)
					{
						errorAt(fstr("%s %s", curWord, idxArray(&words, i + 1, char*)),
							lineNo, "Expected newline after ':'.");
						return ASSEMBLE_INVALID_MACRO;
					}

					macro = true;
					return ASSEMBLE_SUCCESS;
				}
			} // end of arg parsing

			/*
			now that we know if we're dealing with a macro or not
			we continue parsing accordingly
			*/

			if (macro)
			{
				// TODO: Compile/parse macro
				// printf("\nmacro: '%s'\nargs: ", firstWord);
				// for (int i = 0; i < args.used; i++)
				// 	printf("'%s' ", idxArray(&args, i, char*));
				// printf("\n");
			}
			else // instruction
			{
				char *name = idxArray(&words, 0, char*);
				printf("\n== %s ==\n", name);

				// skip tabs
				char *equals = strpstr(idxArray(&words, i, char*), " ");
				while (true)
				{
					equals = strpstr(equals, "\t");
					if (strlen(equals) == 0)
					{
						i++;
						equals = idxArray(&words, i, char*);
					}
					else break;
				}

				
				// match '='
				if (strcmp("=", equals) != 0)
				{
					errorAt(equals, lineNo, "Expected '='.");
					return ASSEMBLE_INVALID_DECL;
				}

				i++;

				// format
				while (true)
				{
					// TODO: Check if there are more words
					// 		 if not, newline + tab + rest is allowed

					char *curWord = idxArray(&words, i, char*);
					int opcode = consumeIntMaybe(curWord);
					
					if (opcode == INVALID_CONSUME)
						return ASSEMBLE_INVALID_DECL;
					
					printf("-> %d\n", opcode);

					// Instruction instr = newInstruction(opcode, name);

					break;
				}

			}

		}
	}

	return ASSEMBLE_SUCCESS;
}

AssembleStatus assemble(Options *options, bool _verbose,
	const char *instr_path, const char *asm_path,
	const char *instr_src, const char *asm_src)
{
	verbose = _verbose;

	curFile = cpystr(instr_path, strlen(instr_path));
	AssembleStatus instrStatus = compileInstrFile(options, instr_src);
	return instrStatus;
}