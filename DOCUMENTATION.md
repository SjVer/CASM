# CASM Documentation

Contents:</b>
1. [What is CASM?](#what-is-casm)
2. [Instruction Files](#instuction-files)
3. [Assembly Files](#assembly-files)

### What Is CASM?
CASM, or Customisable Assembler is an assembler that uses user-defined instruction sets, macros and output options to assemble a given assembly file into binary or textual machine code.
</b>


### Instuction Files
In order for the assembler to generate proper output it needs an instruction file (by convention with the `.instr` extension). Such a file typically has the following layout:
- [Options](#options)
- [Instructions](#instructions)
- [Macros](#macros)
</b>

#### Options
Options are set using the the following syntax: `#<name> <value(s)>`. During the assembling stage these options (or their default values if not specified) are used to generate the output.
</b>
These are the available options and their default values:
<br> <br>
Binary output mode:
<br>

| Name | Type/Value | Default | Description |
| --- | --- | --- | --- |
| bits | integer | 8 | the amount of bits per outputted value/instruction |
| ext | string | "" | the extension of the outputted file (if no name is specified with the -o cli argument) |
| type | "text"/"bin" | "bin" | the type of output (binary or text) |
| pad | "leading"/"trailing" | "trailing" | wether the values that do not take up the specified amount of bits will be padded with zeros at the front or at the back |
| prefix | integers | | the values that will come before the assembled code |
| suffix | integers | | the values that will come after the assembled code |

Text output mode:

| Name | Type/Value | Default | Description |
| --- | --- | --- | --- |
| bits | integer | 8 | the amount of bits per outputted value/instruction |
| ext | string | "" | the extension of the outputted file (if no name is specified with the -o cli argument) |
| format | string | "{#x} " | the format used to generate the output ([see more](#formats))
| type | "text"/"bin" | "bin" | the type of output (binary or text) |
| pad | "leading"/"trailing" | "trailing" | wether the values that do not take up the specified amount of bits will be padded with zeros at the front or at the back |
| prefix | string | "" | the text that will come before the assembled code |
| suffix | string | "" | the text that will come after the assembled code |

It is important to keep in mind that when setting an option anywhere in the file, the assembler will continue assembling using the new options. In other words: setting options like `bits`, `type`, or any other option that has direct influence on the compilation process in the wrong order or e.g. after the declaration of an instruction might very well result in corrupted output. It is reccommended to set all options at the beginning of the file, starting with the `type` option.

#### Formats
The option `format` can be used (when in text mode) to specify how the assembler will format the compiled instructions/values. In order to give the user as much freedom as possible the format option comes with a lot of features. </b>
The assembler replaces format specifiers (enclosed in curly braces) in the format using the specified format. (That's a lot of "format"'s...). A typical example of such a format is this: `"{#x}. "`. This format contains only one specifier: `#x`. This specifier tells the assembler to output the value as a lowercase hexadecimal with prefix (turning e.g. 15 into "0x0f. ", assuming the bits option is set to 8). </b>
There are a number of these specifiers:

| Specifier | Output |
| --- | --- |
| b | binary |
| #b | binary with prefix "0b" |
| d | decimal |
| #d | decimal with prefix "0d" |
| x | lowercase hexadecimal |
| #x | lowercase hexadecimal with prefix "0x" |
| X | uppercase hexadecimal |
| #X | uppercase hexadecimal with prefix "0X" |

A format can contain an arbitrary amount of these specifiers.

#### Instructions
The most important part of any instruction file are its instructions. These are declared using the following syntax: `<name> <arguments> = <constants and arguments>`. The name can be any sequence of characters except spaces, tabs and newlines.
<br>
The arguments have a syntax of their own: `{<name>:<bits>}` where `<bits>` is the width in bits of said argument. If -for example- an argument named "address" would be a byte long, the syntax would be `{address:8}`.
<br/>
The next part -the constants and arguments- requires the constants and/or arguments to be seperated by a `+`. Constants will be part of the assembled instruction/value just the way they are while arguments are replaced by the value given by the user in the assembly file. An example of this is the following: ``0x3`8 + address``.
<br>
Here, the assembled instruction/value will consist of the constant 3 in 8 bits followed by whatever argument is given as address in the amount of bits specified earlier. (In the case of `{address:8}` that's 8 bits.)
</b>
It is worth noting that if a constant is defined without size (a backtick followed by an integer) the assembler will automatically pick the lowest amount of bits possible. This might not be convinient so it is reccommended to always specify the width of a constant.
<br>
Combining all this knowledge a simple "load address" instruction could be defined as ``lda {address:8} = 0x3`8 + address``.
If you were to set the type option to binary, the bits option to 16 and assemble `    lda 5`, the output would be `0x0305`.

#### Macros
W.I.P.

### Assembly Files
Where the instruction file defines how the assembler will compile input, the assembly file is the actual input. The syntax of CASM assembly is rather simple:
</b>
instructions are indented by a single tab, and any arguments are given seperated by commas. Labels are declared undindented, consisting of a single word and ending in a colon. These labels can be used as arguments throughout the entire assembly file and will be replaced by the address of the first instruction after the declaration.