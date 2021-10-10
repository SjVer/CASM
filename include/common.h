#ifndef COMMON_H
#define COMMON_H

#define  _GNU_SOURCE

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>

#pragma GCC diagnostic ignored "-Wcomment"
#pragma GCC diagnostic ignored "-Wunused-variable"

#include "tools.h"

// OS name
#ifdef _WIN32
#define OS "Windows 32-bit"
#elif _WIN64
#define OS "Windows 64-bit"
#elif __APPLE__ || __MACH__
#define OS "Mac OSX"
#elif __linux__
#define OS "Linux"
#elif __FreeBSD__
#define OS "FreeBSD"
#elif __unix || __unix__
#define OS "Unix"
#else
#define OS "Other"
#endif

typedef enum
{
    ASSEMBLE_SUCCESS,
    ASSEMBLE_INVALID_MACRO,
    ASSEMBLE_INVALID_OPTION,
    ASSEMBLE_INVALID_DECL,
    ASSEMBLE_INVALID_INSTR,
    ASSEMBLE_INVALID_ARGS,
    ASSEMBLE_INVALID_LABEL
} AssembleStatus;



typedef enum
{
    SPEC_BIN,
    SPEC_DEC,
    SPEC_HEX_LOW,
    SPEC_HEX_UP,
} SpecifierType;

static char specifiers[] = {
    [SPEC_BIN] = 'b',
    [SPEC_DEC] = 'd',
    [SPEC_HEX_LOW] = 'x',
    [SPEC_HEX_UP] = 'X',
};

#define specifierCount 4

typedef struct
{
    SpecifierType type;
    bool prefix;
} Specifier;

typedef struct
{
    int count;
    Array specifiers, betweens;
    char *after;

    /* example: " 0x{x} - {#b}."
        count = 2;
        specifiers = {
            { SPEC_HEX_LOW; false; },
            { SPEC_BIN;     true; },
        };
        strs = {
            " 0x",
            " - ",
        };
        after = ".";
    */
} Format;



// app info
#define APP_NAME "casm"
#define APP_VERSION "0.0.1"
#define APP_DOC APP_NAME \
" -- A fully customiseable assembler for your own instruction sets.\n\
Written by Sjoerd Vermeulen ("EMAIL")\v\
More info at https://github.com/SjVer/CASM.\n\
Build: " __DATE__ " " __TIME__ " on " OS " (" COMPILER ")."

// misc info
#define EMAIL "sjoerd@marsenaar.com"

// compiler stuff
#define DEFAULT_BITWIDTH 8

#include "chunk.h"
#include "compiler.h"

#endif