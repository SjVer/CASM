#include "common.h"
#define INVALID_CONSUME -1

static Instruction newInstruction(Array Args_array, int argc, const char *name)
{
	Instruction instr;
	instr.argc = argc;
	instr.name = cpystr(name, strlen(name));
	instr.args = Args_array;
	return instr;
}

void initOptions(Options *options)
{
	options->bitwidth = DEFAULT_BITWIDTH;
	options->outputType = TYPE_BINARY;
	options->extension = cpystr("", 0);
	options->padLeading = false;
	
	options->format = NULL;
	options->formatLen = 0;
	options->prefix = NULL;
	options->prefixLen = 0;
	options->suffix = NULL;
	options->suffixLen = 0;
}

// =============================

char *curFile;
int verbose;
Array instructions, definedLabels, tmpLabels;

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

static bool verifyFormat(const char* format)
{
	bool ISSPEC(char ch)
	{
		for (int i = 0; i < specifierCount; i++)
			if (ch == specifiers[i]) return true;
		return false;
	}

	int i = 0;
	while (i < strlen(format))
	{
		// cannot have '}' before '{'
		if (format[i] == '}') return false;

		if (format[i] == '{')
		{
			// at least two more chars (specifier and '}')
			if (i + 2 >= strlen(format)) return false;
			i++; // char is now the one after '{'

			if (!(ISSPEC(format[i]) || format[i] == '#')) return false;
			
			// if '#' then a specifier is a must
			if (format[i] == '#')
			{
				i++;

				// specifier and at least two more chars (specifier and '}')
				if (i + 1 >= strlen(format) || !ISSPEC(format[i])) return false;
				i++;

				// expect '}'
				if (format[i] != '}') return false;
			}
			i++;

		}

		i++;
	}
	return true;
}

// =============================

typedef void* (*consumeFn)(const char *part, const char *name, bool inList);
char *curLine;
int lineNo;

static void *consumeIntMacro(const char *part, const char *name, bool inList)
{
	char *str = strpstr(part, " ");

	// hexadecimal
	if (strstart(str, "0x"))
	{
		for (int i = 2; i < strlen(str); i++)
			if (!(tolower(str[i]) >= '0' && tolower(str[i]) <= 'f'))
			{	
				errorAt(curLine, lineNo, fstr("'%s' is not a valid hexadecimal number.", str));
				return (void *)INVALID_CONSUME;
			}
		return ADDR(int, strtol(str + 2, NULL, 16));
	}

	// binary
	else if (strstart(str, "0b"))
	{
		for (int i = 2; i < strlen(str); i++)
			if (str[i] != '0' && str[i] != '1')
			{	
				errorAt(curLine, lineNo, fstr("'%s' is not a valid hexadecimal number.", str));
				return (void *)INVALID_CONSUME;
			}
		return ADDR(int, strtol(str + 2, NULL, 2));
	}

	// decimal
	else if (isnum(str, false)) return ADDR(int, atoi(part));

	errorAt(curLine, lineNo, fstr("%s expects %s, not '%s'.", 
		name, inList ? "integers" : "an integer", part));
	return (void *)INVALID_CONSUME;
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
					options->prefixLen = args.used;

					char *msg = "Prefix set to '";
					for (int i = 0; i < args.used; i++)
						msg = fstr("%s%s0x%x'", msg, i == 0 ? "" : "\b ", prefix[i]);

					msgAt(lineNo, msg);
				}
				else if (options->outputType == TYPE_TEXT)
				{
					char *prefix = consumeStr(arg, "option 'prefix'", false);
					if (prefix == (char *)INVALID_CONSUME) return ASSEMBLE_INVALID_OPTION;
					
					options->prefix = (int *)prefix;
					options->prefixLen = strlen(prefix);
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

					options->suffix = (int *)suffix;
					options->suffixLen = args.used;

					char *msg = "Suffix set to '";
					for (int i = 0; i < args.used; i++)
						msg = fstr("%s%s0x%x'", msg, i == 0 ? "" : "\b ", suffix[i]);

					msgAt(lineNo, msg);
				}
				else if (options->outputType == TYPE_TEXT)
				{
					char *suffix = consumeStr(arg, "option 'suffix'", false);
					if (suffix == (char *)INVALID_CONSUME) return ASSEMBLE_INVALID_OPTION;
					
					options->suffix = (int *)suffix;
					options->suffixLen = strlen(suffix);
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

					if (!verifyFormat(format))
					{
						errorAt(curLine, lineNo, "invalid format.");
						return ASSEMBLE_INVALID_OPTION;
					}

					options->format = (int *)format;
					options->formatLen = strlen(format);
					msgAt(lineNo, fstr("Format set to '%s'.", options->format));
				}
			}

			// #pad {"trailing"|"leading"}
			else if (strstart(curLine + 1, "pad"))
			{
				char *arg = strpstr(curLine + 5, " ");

				if (strcmp(arg, "trailing") == 0)
					options->padLeading = false;
				else if (strcmp(arg, "leading") == 0)
					options->padLeading = true;
				
				else
				{
					errorAt(curLine, lineNo, 
						fstr("option 'pad' expects 'leading' or 'trailing', not '%s'.", arg));
					return ASSEMBLE_INVALID_OPTION;
				}

				msgAt(lineNo, fstr("Pad mode set to %s.", arg));
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
				
				// seperate and handle name and bitwidth
				Array parts = spltstr(cpystr(curWord + 1, strlen(curWord) - (strend(curWord, ":") ? 3 : 2)), ":");
				if (parts.used != 2)
				{
					errorAt(curWord, lineNo, "Expect '{name:bitwidth}'.");
					return ASSEMBLE_INVALID_DECL;
				}
				
				// create arg
				Arg arg;
				arg.bitwidth = *(int *)consumeIntMacro(idxArray(&parts, 1, char*), "argument bitwidth", false);
				if (arg.bitwidth == INVALID_CONSUME) return ASSEMBLE_INVALID_DECL;
				arg.name = cpystr(idxArray(&parts, 0 , char*), strlen(idxArray(&parts, 0 , char*)));

				// _appendArray(&args, strcpy(malloc(strlen(curWord) + 1), curWord));
				// _appendArray(&argsW, memcpy(malloc(sizeof bitwidth), &bitwidth, sizeof bitwidth));
				// _appendArray(&args, memcpy(malloc(sizeof arg), &arg, sizeof arg));
				appendArrayCpy(&args, arg);

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
				#define CURWORD (strpstr(strpstr(idxArray(&words, i, char*), " "), "\t"))

				char *name = idxArray(&words, 0, char*);

				if (i == words.used)
				{
					// last word, expect newline and then the rest
					lineNo++; i = 0;
					curLine = idxArray(&spltstr(idxArray(&lines, lineNo - 1, char*), ";"), 0, char*);
					words = spltstr(curLine, " ");
				}

				// skip tabs
				char *equals = CURWORD;
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

				// format/arguments

				Array usedArgs = newArray(0, 1, true);
				
				#define PRINTARG(prefix, arg) \
					printf("%sarg.name: '%s'\n%sarg.opcode: %d\n%sarg.bitwidth: %d\n", \
						prefix, arg.name, prefix, arg.opcode, prefix, arg.bitwidth)   \

				while (true)
				{
					if (i == words.used) // allow newline
					{
						// last word, expect newline (tabs?) and then the rest
						lineNo++; i = 0;
						curLine = idxArray(&spltstr(idxArray(&lines, lineNo - 1, char*), ";"), 0, char*);
						words = spltstr(curLine, " ");

						// skip all tabs
						while (strlen(CURWORD) == 0)
							i++;
					}

					char *curWord = CURWORD;
					int opcode = consumeIntMaybe(idxArray(&spltstr(curWord, "`"), 0, char*));
					
					if (opcode == INVALID_CONSUME) // no int, maybe argument?
					{
						// check if argument exists
						Arg arg;
						bool found = false;
						
						for (int j = 0; j < args.used; j++)
						{
							arg = idxArray(args, j, Arg);
							// printf("  arg: %s : %d\n", arg.name, arg.bitwidth);
							if (strcmp(arg.name, curWord) == 0) { found = true; break; }
						}
						
						if (!found) // undefined argument
						{
							errorAt(curWord, lineNo, "undefined argument.");
							return ASSEMBLE_INVALID_DECL;
						}
						
						appendArrayCpy(&usedArgs, arg);
					}
					else
					{
						Arg opcodeArg;
						opcodeArg.name = "";
						if (spltstr(curWord, "`").used != 1)
						{
							Array parts = spltstr(curWord, "`");
							if (parts.used != 2) // invalid bitwidth specifier
							{
								errorAt(curWord, lineNo, "invalid bitwidth specifier.");
								return ASSEMBLE_INVALID_DECL;
							}

							opcodeArg.bitwidth = *(int *)consumeIntMacro(
								idxArray(&parts, 1, char*), "bitwidth specifier", false);
							if (opcodeArg.bitwidth == INVALID_CONSUME) return ASSEMBLE_INVALID_DECL; 
						}
						else opcodeArg.bitwidth = bitlen(opcode);

						opcodeArg.opcode = opcode;

						// PRINTARG("=== ", opcodeArg);
						appendArrayCpy(&usedArgs, opcodeArg);
					}

					i++;
					if (i == words.used) // allow newline
					{
						// if last word is '+' a newline is allowed, otherwise not
						if (strcmp(curWord, "+") != 0) break;

						lineNo++; i = 0;
						curLine = idxArray(&spltstr(idxArray(&lines, lineNo - 1, char*), ";"), 0, char*);
						words = spltstr(curLine, " ");

						// skip all tabs
						while (strlen(CURWORD) == 0)
							i++;						
					}
					else // consume one '+'
					{
						if (strcmp(CURWORD, "+") != 0)
						{
							errorAt(CURWORD, lineNo, "expected '+'.");
							return ASSEMBLE_INVALID_DECL;
						}
						i++;
					}

				}

				// create instruction
				Instruction instr = newInstruction(usedArgs, args.used, name);
				appendArrayCpy(&instructions, instr);

				/*
				printf("\n=== %s ===\n", instr.name);
				for (int i = 0; i < instr.args.used; i++)
				{
					printf("\n");
					Arg arg = idxArray(instr.args, i, Arg);
					if (strlen(arg.name) != 0) printf("name: %s\n", arg.name);
					else printf("opcode: %d\n", arg.opcode);
					printf("bitwidth: %d\n", arg.bitwidth);
				}
				*/

				#undef PRINTARG
				#undef CURWORD
			}

		}
	}

	return ASSEMBLE_SUCCESS;
}

static AssembleStatus assembleAsmFile(Options *options, const char *src, Chunk *chunk)
{
	// split into lines
	Array lines = spltstr(src, "\n");
	definedLabels = newArray(0, 1, true);
	tmpLabels = newArray(0, 1, true);


	for (lineNo = 1; lineNo < lines.used + 1; lineNo++)
	{
		curLine = idxArray(&spltstr(idxArray(&lines, lineNo - 1, char*), ";"), 0, char*);
		curLine = strpstr(curLine, " ");
		Array words = spltstr(curLine, " ");

		#define WORD(i) (strpstr(strpstr(idxArray(&words, i, char*), " "), "\t"))

		if (strstart(curLine, "\t")) // indented thus instruction
		{
			curLine = strpstr(curLine, "\t");
			char *curWord = WORD(0);
			
			Instruction instr;

			// search instructions for match
			bool found = false;
			for (int i = 0; i < instructions.used; i++)
			{
				if (strcmp(curWord, idxArray(instructions, i, Instruction).name) == 0)
				{
					instr = idxArray(instructions, i, Instruction);
					found = true;
					break;
				}
			}

			if (!found)
			{
				// errorAt(curWord, lineNo, "undefined instruction.");
				// return ASSEMBLE_INVALID_INSTR;
			}
			else
			{
				//// printf("\n=== %s (%d) ===\n", instr.name, instr.argc);
				
				typedef struct { int value; int bitwidth; } Bits;
				Array allBits = newArray(0, 1, true);
				int totalBitCount = 0;

				int wordNo = 1;
				for (int argNo = 0; argNo < instr.args.used; argNo++)
				{
					Arg curArg = idxArray(instr.args, argNo, Arg);
					
					// check if constant -> just write it and continue
					if (strlen(curArg.name) == 0)
					{
						//// printf("const %d: %d (%d)\n", argNo, curArg.opcode, curArg.bitwidth);
						Bits bits;
						bits.value = curArg.opcode;
						bits.bitwidth = curArg.bitwidth;
						appendArrayCpy(&allBits, bits);

						totalBitCount += bits.bitwidth;
						continue;
					}

					// we now know we're dealing with an argument

					// check if no words left -> error
					if (wordNo >= words.used)
					{
						errorAt(curLine, lineNo, fstr("Expected %d arguments.", instr.argc));
						return ASSEMBLE_INVALID_ARGS;
					}

					curWord = WORD(wordNo);
					//// printf("arg %d: '%s' (%d), ", argNo, curArg.name, curArg.bitwidth);

					// handle arg
					int result = consumeIntMaybe(strpstrb(curWord, ","));
					if (result == INVALID_CONSUME) // tmpLabel stuff
					{
						// we add the label to the tmpLabels and check in the end if it is valid
						curWord = strpstrb(curWord, ",");

						TmpLabel label;
						label.address = chunk->count;
						label.bitpos = totalBitCount;
						label.bitwidth = curArg.bitwidth;
						label.line = lineNo;
						label.name = cpystr(curWord, strlen(curWord));

						appendArrayCpy(&tmpLabels, label);
						totalBitCount += curArg.bitwidth;
					}

					//// printf("parsed: %d\n", result);
					
					if (result == INVALID_CONSUME) result = 0xff; // temp


					// write parsed argument
					{
						Bits bits;
						bits.value = result;
						bits.bitwidth = curArg.bitwidth;
						appendArrayCpy(&allBits, bits);

						totalBitCount += curArg.bitwidth;
					}

					wordNo++;

					// possibly skip one ',' if it wasn't at the end of the previous word
					if (argNo < instr.argc && !strend(curWord, ","))
					{
						if (wordNo >= words.used || (strcmp(WORD(wordNo), ",") != 0))
						{
							errorAt(curLine, lineNo + 1, fstr("Expected %d arguments seperated by commas.", instr.argc));
							return ASSEMBLE_INVALID_ARGS;
						}
						wordNo++;
					}
				}

				// no more words should be left by now
				if (wordNo != words.used)
				{
					errorAt(WORD(wordNo), lineNo + 1, fstr("Expected %d arguments.", instr.argc));
					return ASSEMBLE_INVALID_ARGS;
				}

				// convert parsed argument to int
				int out;
				{

					// convert to binary string
					char *bitsstr = "";
					for (int i = 0; i < allBits.used; i++)
					{
						Bits bits = idxArray(allBits, i, Bits);
						bitsstr = fstr("%s%s", bitsstr, bitsf(bits.value, bits.bitwidth));
					}

					// pad with trailing zeros
					while (strlen(bitsstr) < options->bitwidth)
						bitsstr = options->padLeading ? fstr("0%s", bitsstr) : fstr("%s0", bitsstr);

					//// printf("output: 0b%s -> ", bitsstr);

					out = strtol(bitsstr + 2, NULL, 2);
					//// printf("0x%.*x (chunk->count = %d)\n", 
					//// 	options->bitwidth / 4, out, chunk->count);
					//// printf("=== ======== ===\n\n");
				}
				writeChunk(chunk, out, lineNo);
			}
		}
		
		else // label or ...?
		{
			char *curWord = WORD(0);

			if (strend(curWord, ":")) // check if label
			{
				if (words.used > 1)
				{
					errorAt(curLine, lineNo, "invalid label declaration.");
					return ASSEMBLE_INVALID_LABEL;
				}

				Label label;
				label.address = chunk->count + 1;
				label.name = cpystr(curWord, strlen(curWord) - 1);

				appendArrayCpy(&definedLabels, label);
				
				msgAt(lineNo, fstr("Label '%s' defined at address 0x%x.", label.name, label.address));
			}
			else
			{
				// TODO: other things than labels perhaps?
				errorAt(curLine, lineNo, "invalid statement. (Expected indent or label.)");
				return ASSEMBLE_INVALID_LABEL;
			}
		}
	}

	// replace all temp labels
	for (int i = 0; i < tmpLabels.used; i++)
	{
		TmpLabel label = idxArray(tmpLabels, i, TmpLabel);
		int newval = -1;

		// search for label
		for (int j = 0;j < definedLabels.used; j++)
		{
			Label lab = idxArray(definedLabels, i, Label);
			if(strcmp(lab.name, label.name) == 0)
			{ newval = lab.address; break; }
		}

		// not found?
		if (newval == -1)
		{
			errorAt(label.name, label.line, "undefined label used.");
			return ASSEMBLE_INVALID_LABEL;
		}

		//// printf("!!! replacing %d bits starting at bit %d at address %d with %d (label '%s')\n",
		//// 	label.bitwidth, label.bitpos, label.address, newval, label.name);

		char *bitsstr = bitsf(chunk->bytes[label.address], options->bitwidth);
		//// printf("old bits: %s\n", bitsstr);
	
		char *newbits = bitsf(newval, label.bitwidth);
		strncpy(bitsstr + label.bitpos, newbits, label.bitwidth);

		//// printf("new bits: %s\n", bitsstr);
		chunk->bytes[label.address] = (int)strtol(bitsstr, NULL, 2);
	}

	return ASSEMBLE_SUCCESS;
}

AssembleStatus assemble(Options *options, int _verbose, Chunk *chunk,
	const char *instr_path, const char *asm_path,
	const char *instr_src, const char *asm_src)
{
	verbose = _verbose;
	instructions = newArray(0, 1, true);

	if (verbose > 1) printf("~ Parsing %s...\n", instr_path);

	curFile = cpystr(instr_path, strlen(instr_path));
	AssembleStatus instrStatus = compileInstrFile(options, instr_src);

	if (instrStatus != ASSEMBLE_SUCCESS) return instrStatus;

	if (verbose > 1) // debug message
	{
		printf("~ Parsing %s done.\n~ Instructions:\n", instr_path);
		for (int j = 0; j < instructions.used; j++)
		{
			Instruction instr = idxArray(instructions, j, Instruction);
			printf(" %s:\n", instr.name);
			for (int i = 0; i < instr.args.used; i++)
			{
				Arg arg = idxArray(instr.args, i, Arg);
				if (strlen(arg.name) != 0) printf("   %d: argument: %s, ", i + 1, arg.name);
				else printf("   %d: constant: %d, ", i + 1, arg.opcode);
				printf("bitwidth: %d\n", arg.bitwidth);
			}
		}
	}

	if (verbose > 1) printf("~ Assembling %s...\n", asm_path);

	curFile = cpystr(asm_path, strlen(asm_path));
	AssembleStatus asmStatus = assembleAsmFile(options, asm_src, chunk);

	//// printf("\ndefined labels:\n");
	//// for (int i = 0; i < definedLabels.used; i++)
	//// {
	//// 	Label lab = idxArray(definedLabels, i, Label);
	//// 	printf(" at %d: %s\n", lab.address, lab.name);
	//// }
	//// printf("\n");

	if (verbose > 1) // print chunk
	{
		printf("~ Assembling %s done.\n~ Assembled output:\n", asm_path);
	
		int printed = 0;
		for (int i = 0; i < chunk->count; i++)
		{
			printf(" 0x%.*x", options->bitwidth / 4, chunk->bytes[i]);
			printed++;
			if (printed >= OUTPUT_PRINT_COUNT)
				{ printed = 0; printf("\n"); }
		}
		if (chunk->count == 0) printf("(No output)");
		printf("\n");
	}

	return asmStatus;
}