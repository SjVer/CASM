# CASM
A fully customisable assembler for your own instruction sets!
<br>
1. [What Is CASM?](#-what-is-casm?) 🇨
2. [Documentation](#-documentation) 📚
3. [Command-Line Usage](#-command-line-usage) 💻
4. [How To Install CASM](#-how-to-install-casm) 💾

### 🇨 What Is CASM?
CASM, or Customisable Assembler is an assembler that uses user-defined instruction sets, macros and output options to assemble a given assembly file into binary or textual machine code.

### 📚 Documentation
Check out the documentation [here](DOCUMENTATION.md)!

### 💻 Command-Line Usage
Usage:
<pre>
casm [-v?V] [-o OUTFILE] [--output=OUTFILE] [--verbose] [--help]
    [--usage] [--version] INSTRFILE ASMFILE
</pre>
Options:
<pre>
  -o, --output=OUTFILE       Output to OUTFILE instead of to standard output
  -v, --verbose              Produce verbose output (-vv for extra verbosity)
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version
</pre>
Examples:
<pre>
$ casm z80.instr kernel.asm
$ casm -v z80.instr kernel.asm -o mykernel
$ casm -vv -w z80.instr kernel.asm -o mykernel
</pre>

### 💾 How To Install CASM

#### 🐧 Linux

<b>Step 1:</b>
Download the debian package:

| | |
| --- | --- |
| x86_64 | [casm_0.1_x86-64.deb](https://github.com/SjVer/CASM/releases/download/0.1/casm_0.1_x86-64.deb) |
| armv7l | [casm_0.1_armv7l.deb](https://github.com/SjVer/CASM/releases/download/0.1/casm_0.1_armv7l.deb) |

<br>
<b>Step 2:</b>
Go to the directory where you downloaded the `.deb` file and run the following command:
```console
$ sudo apt install ./downloadedfile.deb
```