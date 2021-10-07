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

It is important to keep in mind that when setting an option anywhere in the file, the assembler will continue assembling using the new options. In other words: setting options like `bits`, `type`, or any other option that has direct influence on the compilation process in the wrong order or e.g. after the declaration of an instruction might very well result in corrupted output.

#### Formats
The option `format` can be used (when in text mode) to specify how the assembler will format the compiled instructions/values. In order to give the user as much freedom as possible the format option comes with a lot of features. </b>
The assembler replaces format specifiers (enclosed in curly braces) in the format using the specified format. (That's a lot of "format"'s...). A typical example of such a format is this: `"{#x} "`. This format contains only one specifier: `#x`. This specifier tells the assembler to output the value as a lowercase hexadecimal with prefix (turning e.g. 15 into "0x0f", assuming the bits option is set to 8). 


### Assembly Files
stuff