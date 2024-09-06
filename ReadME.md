# POSIX Shell Implementation

**Course**: Advanced Operating Systems 
**Assignment**: POSIX Shell Implementation  

## Overview

This project is a custom POSIX-compliant shell written in C++. The shell provides interface with support for executing commands, managing background processes, handling input/output redirection, piping, and implementing built-in commands like `cd`, `pwd`, `echo`, `ls`, `pinfo`, and `history`. The shell also handles basic signal processing, command history, and tab completion.

## Features

### 1. Shell Prompt
The shell prompt is dynamic and displays in the format:
The `~` symbol represents the home directory.

### 2.  Commands
- **`cd`**: Change the current directory. Supports flags like `.`, `..`, `-`, and `~`.
- **`pwd`**: Print the current working directory.
- **`echo`**: Display a line of text.
- **`ls`**: List directory contents. Supports flags `-a`, `-l`, and various combinations.
- **`pinfo`**: Display process information. Supports querying by PID.
- **`history`**: Display command history. The shell maintains a history across sessions.

### 3. Command Execution
- **Foreground and Background Execution**: Supports running commands in both foreground and background using `&`.
- **System Commands**: All non-built-in commands are executed as system commands.
  
### 4. Input/Output Redirection
- **Input Redirection (`<`)**: Redirect input from a file.
- **Output Redirection (`>`)**: Redirect output to a file (overwrite).
- **Append Redirection (`>>`)**: Append output to a file.

### 5. Piping
- **Pipe (`|`)**: Redirect the output of one command as input to another. Supports multiple pipes in a single command.

### 6. Signal Handling
- **CTRL-Z**: Move the foreground process to the background and stop it.
- **CTRL-C**: Interrupt the foreground process.
- **CTRL-D**: Exit the shell.

### 7. Command History
- Stores up to 20 commands across sessions.
- **Arrow Keys**: Navigate through the command history using the UP and DOWN arrow keys.

### 8. Autocompletion
- **Tab Completion**: Autocompletes displays possible filenames based on the current directory's contents.

### 9. Search
- **Search**: Recursively search for a file or folder under the current directory.

## Directory Structure

## Project Structure
- `main.cpp`: Entry point of the shell.
- `user_env.h` / `user_env.cpp`: Contains functions related to system details like username, hostname, etc.
- `auto_complete.h` / `auto_complete.cpp`: Implements the autocomplete feature.
- `cd.cpp`: Implements the `cd` command functionality.
- `echo.cpp`: Implements the `echo` command functionality.
- `fg.h` / `fg.cpp`: Manages foreground and background processes.
- `history.h` / `history.cpp`: Implements command history management.
- `ls.cpp`: Implements the `ls` command functionality.
- `pinfo.h` / `pinfo.cpp`: Implements the `pinfo` command functionality.
- `raw_mode.h` / `raw_mode.cpp`: Handles terminal raw mode settings for capturing keypresses.
- `search.cpp`: Implements the recursive search functionality.

## Compilation Instructions

### Unzipping the Project
First, unzip the project folder using the following command:

- unzip <downloaded_zip>
- If you unzipped in desktop
- ```
     cd ./desktop/<downloaded_zip>
     ```

- To compile the shell, follow these steps:

- ```bash
     make clean  # Deletes previous .o files
     make        # Compiles the shell
    ./shell      # Start the Shell
