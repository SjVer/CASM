#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#pragma GCC diagnostic ignored "-Wcomment"

typedef enum
{
    ASSEMBLE_SUCCESS,
    ASSEMBLE_INVALID_MACRO,
    ASSEMBLE_INVALID_OPTION,
    ASSEMBLE_INVALID_DECL,
    ASSEMBLE_INVALID_INSTR,
    ASSEMBLE_INVALID_ARGS
} AssembleStatus;

// app info
#define APP_NAME "casm"
#define APP_VERSION "0.0.1"
#define APP_DOC APP_NAME \
" -- A fully customiseable assembler for your own instruction sets.\n\
Written by Sjoerd Vermeulen ("EMAIL")\v\
More info at https://github.com/SjVer/CASM."

// misc info
#define EMAIL "sjoerd@marsenaar.com"

// compiler stuff
#define DEFAULT_BITWIDTH 8

#include "tools.h"
#include "chunk.h"
#include "compiler.h"

#endif