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
</b>
| Name | Binary/Text mode | Default (Binary) | Default (text) | Description |
| --- | --- | --- | --- | --- |
| bits | both | 8 | 8 | the amount of bits per outputted value/instruction |
| ext | both | "" | "" | the extension of the outputted file (if none specified) |

### Assembly Files
stuff