[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/u16ttUuk)
# CSEShell

CSEShell is a simple, custom shell for Unix-based systems, designed to provide an interface for executing system programs. This project includes a basic shell implementation, a set of system programs (`find`, `ld`, `ldr`), and some test files.

## Directory Structure

The project is organized as follows:

- `bin/` - Contains compiled executables for system programs.
  - `find` - Program to find files.
  - `ld` - Program for listing the contents of the current directory.
  - `ldr` - Program for listing the contents of the current directory recursively.
- `cseshell` - The main executable for the CSEShell.
- `files/` - Contains various test files used with the shell and system programs.
  - `combined.txt`, `file1.txt`, `file2.txt`, ... - Test text files.
  - `notes.pdf` - A PDF file for testing.
  - `ss.png` - An image file.
- `makefile` - Makefile for building the CSEShell and system programs.
- `source/` - Source code for the shell and system programs.
  - `shell.c` and `shell.h` - Source and header files for the shell.
  - `system_programs/` - Source code and header for the system programs.

## Building the Project

To build the CSEShell and system programs, run the following command in the root directory:

```bash
make
```

This will compile the source code and place the executable files in the appropriate directories. Note that the initial setup also requires changing `PROJECT_DIR` and `PATH` in the .cseshellrc file and the `output_file_path` in dspawn.c line 11.

## Running CSEShell

After building, you can start the shell by running:

```bash
./cseshell
```

From there, you can execute built-in commands and any of the included system programs (e.g., `find`, `ld`, `ldr`).

## Builtin Commands

- `cd`- Changes the current directory of the shell to the specified path. If no path is given, it defaults to the user's home directory.
- `help` - List all builtin commands in the shell.
- `exit` - Exits the shell.
- `usage` - Provides a brief usage guide for the shell and its built-in command.
- `env` - Lists all the environment variables currently set in the shell.
- `setenv` - Sets or modifies an environment variable for this shell session.
- `unsetenv` - Removes an environment variable from the shell.

## System Programs

- `find.c` - Searches for files in a directory.
- `ld.c` - List the contents of the curent directory.
- `ldr.c` - List the contents of the current directory recursively.
- `sys.c` - List the basic information about the operating system.
- `dspawn.c` - Summons a daemon process and terminates to allow the shell to display the next prompt.
- `dcheck.c` - List the number of live daemons (spawned from `dspawn`).
- `backup.c` - Zips a file or directory, specified by `BACKUP_DIR`, into another directory specified by `PROJECT_DIR`.

Each program can be executed from the CSEShell once it is running. This starter code only allows the shell to execute a command once before exiting because `execv` replace the entire process' address space. We have fixed this and allowed the shell to prompt for more commands without having to exit the shell as well as added more system programs (e.g., `sys`, `dspawn`, `dcheck`, `backup`).

## Files Directory

The `files/` directory contains various text, PDF, and image files for testing the functionality of the CSEShell and its system programs.

## Makefile

The Makefile contains rules for compiling the shell and system programs. You can clean the build by running:

```bash
make clean
```

## Inclusivity Feature
There is a customisation feature for users such that they are able to change the text: color, toggle between bold and normal text as well as prompt styles. 

This shell supports 7 extra colors other than the default(light) : red, green, blue, yellow, magenta, cyan, and dark
2 text types: normal and bold
infinity prompt styles as it is designed by you! 

This allows users to have a unique command line experience as they are abe to choose the color and font density to be able to see clearly or just have fun with the different styles available!

cmd line inputs
```
set-color-scheme [colour]
set-text-size [normal/bold]
set-prompt-style [custom_input] (only characters before space is captured)

```

## Sustainability Feature
#### Idle Resource Management
This feature reduces resource usage when the shell is idle. When there is no user input for more than 10 sec, the shell will enter low-power mode. In low-power mode, the shell will pause background processes that are not essential (if any) and then sleep. Only when new command is entered, then the shell will be woken up.

This contributes to energy efficiency as by reducing resource usage during idle times, the shell conserves energy. Moreover, this feature results in optimized performance of the shell. By pausing non-essential background processes, the system can allocate more resources to tasks when they are needed, potentially improving performance and responsiveness when the shell is active.

## Source Directory

Contains all the necessary source code for the shell and system programs. It is divided into the shell implementation (`shell.c`, `shell.h`) and system programs (`system_programs/`).

## Team members
- Ng Wan Qing
- Shivani Mariappan
- Ho Atsadet
