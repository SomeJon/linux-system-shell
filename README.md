# Custom Linux Shell (C)

A command-line shell written in C on top of low-level POSIX system calls. It runs user commands in child processes, chains commands with pipes, and redirects input and output.

Built as part of an Operating Systems course.

## Features

- **Process management**: each command runs in a child process using `fork()`, `execvp()` and `waitpid()`, while the parent shell keeps running.
- **Pipes**: chains up to three commands (`cmd1 ! cmd2 ! cmd3`) with `pipe()`, connecting each command's output to the next one's input.
- **I/O redirection**: input (`}`) and output (`{`) redirection with `dup2()` on `STDIN_FILENO` / `STDOUT_FILENO`.
- **Syntax validation**: a custom parser checks each command line before running it, and rejects invalid redirections and malformed pipes.
- **Clean exit**: the shell exits its main loop on EOF (Ctrl+D).

## Implementation notes

- Unused pipe ends are closed in both the parent and child processes, so that EOF propagates correctly and no process blocks waiting on an open pipe.
- Process creation, piping and redirection are done with direct system calls rather than higher-level library wrappers such as `system()` or `popen()`.

## Project structure

- `os2.c`: the full source, including the tokenizer, the command validator and the execution engine.

## Build and run

```bash
gcc os2.c -o myshell
./myshell
```
