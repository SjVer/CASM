[cli-version]: 0.0.1
[file-version]: 0.1

# CASM
A fully customisable assembler for your own instruction sets!
<br>
1. [What Is CASM?](#-what-is-casm?) 🇨
2. [Documentation](#-documentation) 📚
3. [Command-Line Usage](#-command-line-usage) 💻
4. [How To Install CASM](#-how-to-install-casm) 💾
5. [Building CASM](#-building-casm) 🔨

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

NOTE: CASM HAS NOT YET BEEN RELEASED. <br>
If you want to try it anyway you can always clone this repository and build it yourself.

#### 🐧 Linux

<b>Step 1:</b>
Download the debian package:

| | |
| --- | --- |
| x86_64 | [casm_[file-version]_x86-64.deb](https://github.com/SjVer/CASM/releases/download/0.1/casm_[file-version]_x86-64.deb) |
| armv7l | [casm_[file-version]_armv7l.deb](https://github.com/SjVer/CASM/releases/download/0.1/casm_[file-version]_armv7l.deb) |

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

<br>

### 🔨 Building CASM
If you want to build CASM yourself, here's how to do it (on linux):

```sh
$ git clone https://github.com/SjVer/CASM.git
Cloning into 'CASM'...
...

$ cd ./CASM
$ make
[01/5] compiling compiler.c into compiler.o. done!
[02/5] compiling chunk.c into chunk.o. done!
[03/5] compiling cwalk.c into cwalk.o. done!
[04/5] compiling tools.c into tools.o. done!
[05/5] compiling main.c into main.o. done!
[final] compiling final product casm. done!
$ bin/casm --version
casm [cli-version]
```

The CASM executable should be in the `bin` directory. 