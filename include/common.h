#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "tools.h"
#include "chunk.h"

// app info
#define APP_NAME "casm"
#define APP_VERSION "0.0.1"
#define APP_DOC APP_NAME \
" -- A fully customiseable assembler for your own instruction sets.\n\
Written by Sjoerd Vermeulen ("EMAIL")\v\
More info at https://github.com/SjVer/CASM."

// misc info
#define EMAIL "sjoerd@marsenaar.com"

#endif