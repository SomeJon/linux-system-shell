# POSIX-Compliant Custom Linux Shell
A functional Command Line Interface (CLI) developed in C that implements core shell behaviors using low-level POSIX system calls. This project demonstrates a deep understanding of process synchronization, signal handling, and Inter-Process Communication (IPC).

## 🛠️ Technical Implementation
* [cite_start]**Process Lifecycle Management:** Utilizes the `fork()`, `execvp()`, and `waitpid()` pattern to execute user commands in isolated child processes while maintaining a stable parent shell[cite: 126, 135].
* **Advanced Command Piping:** Supports multi-stage command chaining (up to 2 levels) using `pipe()`. [cite_start]This enables data flow between processes (e.g., `cmd1 ! cmd2 ! cmd3`) by redirecting standard outputs to standard inputs[cite: 134, 136].
* [cite_start]**I/O Redirection:** Implements custom input (`}`) and output (`{`) redirection using `dup2()` and low-level file descriptors (`STDIN_FILENO`, `STDOUT_FILENO`)[cite: 129, 137].
* [cite_start]**Robust Error Handling:** Features a custom parser that validates command syntax before execution, preventing invalid redirections or improperly structured pipes[cite: 139, 140].

## 🛡️ Key Systems Knowledge
* [cite_start]**IPC & File Descriptors:** Managed the complexity of closing unused pipe ends in both parent and child processes to avoid deadlocks and ensure proper EOF propagation[cite: 138].
* [cite_start]**Signal Awareness:** Designed the shell to handle graceful termination via EOF (Ctrl+D), ensuring a clean exit from the main execution loop[cite: 127, 128].
* [cite_start]**Pure System Programming:** Strictly avoided high-level library functions in favor of direct system calls, demonstrating a "from scratch" approach to systems software[cite: 144].

## 📂 Project Structure
- `os2.c`: The complete source code containing the tokenizer, command validator, and the multi-process execution engine.

## 🚀 How to Run
1. Compile the shell using `gcc`:
   ```bash
   gcc os2.c -o myshell
