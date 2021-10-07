# CASM
A fully customisable assembler for your own instruction sets!
<br>
1. [What Is CASM?](#-what-is-casm?) 🇨
2. [Documentation](#-documentation) 📚
3. [Command-Line Usage](#-command-line-usage) 💻
4. [How To Install CASM](#-how-to-install-casm) 💾

<br>

### 🇨 What Is CASM?
CASM, or Customisable Assembler is an assembler that uses user-defined instruction sets, macros and output options to assemble a given assembly file into binary or textual machine code.

<br>

### 📚 Documentation
Check out the documentation [here](DOCUMENTATION.md)!

<br>

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

<br>

### 💾 How To Install CASM

NOTE: CASM HAS NOT YET BEEN RELEASED. If you want to try it anyway you can always clone this repository and build it yourself.

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

```sh
$ sudo apt install ./<downloaded file>.deb
```
This should install CASM on your device. To confirm the installation run `casm --version`.

<br>

#### 🪟 Windows

Sadly, CASM on Windows is not yet supported. Maybe later!