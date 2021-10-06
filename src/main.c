#include <argp.h>
#include "common.h"
#include "cwalk.h"

// ================= arg stuff =======================

const char *argp_program_version = APP_NAME " " APP_VERSION;
const char *argp_program_bug_address = EMAIL;

#define ARGS_COUNT 2

/* This structure is used by main to communicate with parse_opt. */
struct arguments
{
	char *args[ARGS_COUNT];	 /* INSTRFILE ASMFILE */
	int verbose;			 /* The -v flag */
	char *outfile;			 /* Argument for -o */
	// char *string1, *string2; /* Arguments for -a and -b */
};

static struct argp_option options[] = {
		// longname, shortname, arg, idk, help
		{"verbose", 'v', 0, 0, "Produce verbose output (-vv, -vvv, etc. to increase verbosity)"},
		{"output", 'o', "OUTFILE", 0, "Output to OUTFILE instead of to standard output"},
		{0}
};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	struct arguments *arguments = state->input;

	switch (key)
	{
	case 'v':
		arguments->verbose += 1;
		break;

	case 'o':
		arguments->outfile = arg;
		break;

	case ARGP_KEY_ARG:
	{
		if (state->arg_num >= ARGS_COUNT)
		{
			argp_usage(state);
		}
		arguments->args[state->arg_num] = arg;
		break;
	}
	case ARGP_KEY_END:
	{
		if (state->arg_num < ARGS_COUNT)
		{
			argp_usage(state);
		}
		break;
	}
	default: return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static char args_doc[] = "INSTRFILE ASMFILE";
static char doc[] = APP_DOC;
static struct argp argp = {options, parse_opt, args_doc, doc};

// ===================================================

Format getFormat(const char *format)
{
	Format form;
	form.count = 0;
	form.specifiers = newArray(0, 1, true);
	form.betweens = newArray(0, 1, true);
	form.after = "";

	// if no '{' its just an after
	if (strchr(format, '{') == NULL)
	{
		form.after = escstr(format);
		return form;
	}

	// at least one specifier
	int i = 0;
	char *curbetween = "";
	while (i < strlen(format))
	{
		if (format[i] != '{')
		{
			curbetween = fstr("%s%c", curbetween, format[i]);
			i++;
			continue;
		}

		Specifier spec = { -1, false };

		// inside format!
		i++;
		if (format[i] == '#') // prefix
		{
			spec.prefix = true;
			i++;
		}

		// now comes the specifier
		for (int j = 0; j < specifierCount; j++)
		{
			if (format[i] == specifiers[j])
				{ spec.type = j; break; }
		}

		// skip the specifier and the '}'
		i++; i++;

		// append the new specifier
		appendArrayCpy(&form.specifiers, spec);
		_appendArray(&form.betweens, (void *)escstr(curbetween));
		curbetween = "";
	}

	form.after = escstr(curbetween);

	return form;
}

int main(int argc, char **argv)
{
    // ========= argument stuff =========

	struct arguments arguments;

	/* Set argument defaults */
	arguments.outfile = NULL;
	arguments.verbose = false;

	/* Where the magic happens */
	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	// ========= setting stuff up =========
    
    char *instr_src = readFile(arguments.args[0]);
    char *asm_src = readFile(arguments.args[1]);
	
	Chunk chunk;
	initChunk(&chunk);

    Options options;
    initOptions(&options);

	// ========= compiling stuff =========

    AssembleStatus status = assemble(&options, arguments.verbose, &chunk,
        arguments.args[0], arguments.args[1], instr_src, asm_src);

    if (status != ASSEMBLE_SUCCESS) return status;

	// ========= output stuff =========

	// if no output file is given the output file is input file 
	// but with the extension given in the instruction file or none
	if (arguments.outfile == NULL)
	{
		char *dot, *newname;
		int len;

		dot = strrchr(arguments.args[0], '.');
		if (dot) len = dot - arguments.args[0];
		else len = strlen(arguments.args[0]);
		
		newname = malloc(len + 6);
		
		strncpy(newname, arguments.args[0], len);
		strcpy(newname + len, options.extension);

		arguments.outfile = newname;
	}

	#define PRINTMSG(level, msg) if (arguments.verbose >= level) printf(msg)
	#define PRINTHEXES(array, len, chunk) 								\
		do {							  								\
			int printed = 0;											\
			for (int i = 0; i < len; i++)								\
			{															\
				PRINTMSG(2, 											\
					fstr(" 0x%.*x", options.bitwidth / 4, array[i]));	\
				printed++;												\
				if (printed >= OUTPUT_PRINT_COUNT)						\
					{ printed = 0; printf("\n"); }						\
																		\
				writeChunk(chunk, array[i], -1);						\
			}															\
		} while (0);

	// write shit
	switch (options.outputType)
	{
		case TYPE_BINARY:
		{
			Chunk newchunk; initChunk(&newchunk);

			// first write prefix
			PRINTMSG(2, "~ Prefix:\n");
			PRINTHEXES(options.prefix, options.prefixLen, &newchunk);
			if (options.prefixLen == 0) PRINTMSG(2, " (No prefix)");
			printf("\n");

			// then write code
			for (int i = 0; i < chunk.count; i++)
				writeChunk(&newchunk, chunk.bytes[i], chunk.lines[i]);

			// then write suffix
			PRINTMSG(2, "~ Suffix:\n");
			PRINTHEXES(options.suffix, options.suffixLen, &newchunk);
			if (options.suffixLen == 0) PRINTMSG(2, " (No suffix)");
			printf("\n");

			uint8_t *buffer = NULL;
			int length = panToUint8Arr(newchunk.bytes, newchunk.count, options.bitwidth, &buffer);
			PRINTMSG(2, fstr("~ Output buffer size: %d bytes\n", length));

			// write to file
			PRINTMSG(2, fstr("~ Writing to file: '%s'\n", arguments.outfile));

			FILE *outfile = fopen(arguments.outfile, "wb");
			fwrite(buffer, sizeof(uint8_t) * length, 1, outfile);
			fclose(outfile);

			PRINTMSG(2, "~ Writing done\n");
			break;
		}
		case TYPE_TEXT:
		{
			#define CAT(dest, str) dest = fstr("%s%s", dest, str)

			char *buffer = "";

			// first write prefix
			if (arguments.verbose >= 2)
			{
				printf("~ Prefix: ");
				if (options.prefixLen != 0) printf("'%s'\n", (char *)options.prefix);
				else printf("(No prefix)\n");
			}
			if (options.prefixLen) CAT(buffer, escstr((char *)options.prefix));

			// get, interpret and apply format
			Format format = getFormat((char *)options.format);
			{
				PRINTMSG(2, fstr("~ Format: (%zu specifier%s)\n", format.specifiers.used,
					format.specifiers.used > 1 ? "s" : ""));

				// apply format per item in chunk
				for (int i = 0; i < chunk.count; i++)
				{
					int value = chunk.bytes[i];

					for (int j = 0; j < format.specifiers.used; j++)
					{
						// between
						if (i==0) PRINTMSG(2, fstr(" -: '%s'\n", idxArray(&format.betweens, j, char*)));
						CAT(buffer, idxArray(&format.betweens, j, char*));

						// specifier
						if (i==0) PRINTMSG(2, fstr(" %d: ", j + 1));
						Specifier spec = idxArray(format.specifiers, j, Specifier);

						// prefix
						if (spec.prefix) buffer = fstr("%s0%c", buffer, specifiers[spec.type]);

						switch (spec.type)
						{
						case SPEC_BIN:
						{
							if (i==0) PRINTMSG(2, "binary");
							CAT(buffer, bitsf(value, options.bitwidth));
							break;
						}
						case SPEC_DEC:
						{
							if (i==0) PRINTMSG(2, "decimal");
							buffer = fstr("%s%d", buffer, value);
							break;
						}
						case SPEC_HEX_LOW:
						{
							if (i==0) PRINTMSG(2, "hexadecimal (lowercase)");
							buffer = fstr("%s%.*x", buffer, options.bitwidth / 4, value);
							break;
						}
						case SPEC_HEX_UP:
						{
							if (i==0) PRINTMSG(2, "hexadecimal (uppercase)");
							buffer = fstr("%s%.*X", buffer, options.bitwidth / 4, value);
							break;
						}
						}
						if (i==0) PRINTMSG(2, fstr(" (%sprefix)\n", spec.prefix ? "" : "no "));
					}

					if (i==0) PRINTMSG(2, fstr(" -: '%s'\n", format.after));

					CAT(buffer, format.after);
				}
			}

			// write suffix
			if (arguments.verbose >= 2)
			{
				printf("~ Suffix: ");
				if (options.suffixLen != 0) printf("'%s'\n", (char *)options.suffix);
				else printf("(No suffix)\n");
			}
			if (options.suffixLen) CAT(buffer, escstr((char *)options.suffix));

			PRINTMSG(2, fstr("~ Output buffer size: %d characters.\n", strlen(buffer)));
			
			// write to file
			PRINTMSG(2, fstr("~ Writing to file: '%s'\n", arguments.outfile));

			FILE *outfile = fopen(arguments.outfile, "w");
			fwrite(buffer, sizeof(char) * (strlen(buffer) + 1), 1, outfile);
			fclose(outfile);

			PRINTMSG(2, "~ Writing done\n");
			
			break;
		}
	}

	return 0;
}