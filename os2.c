#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

#define STR_SEP " \t\n"
#define MAX_ARGS 100
#define MAX_LINE 1024
#define MAX_ARGS_PER_COMMAND 200 // We were not told how many is the max, i would assume 200 is more than enough

#define false 0
#define true 1

typedef char bool;
typedef struct {
    char *args[MAX_ARGS_PER_COMMAND]; // Array of arguments for execvp (e.g., {"ls", "-l", NULL})
    char *inputFile;                  // Filename for input redirection (e.g., "input.txt")
    char *outputFile;                 // Filename for output redirection (e.g., "output.txt")
    int num_args;
} Command;


void init_command_struct(Command *cmd) {
    if (cmd == NULL) return;
    cmd->inputFile = NULL;
    cmd->outputFile = NULL;
    cmd->num_args = 0;
    for (int k = 0; k < MAX_ARGS_PER_COMMAND; k++) {
        cmd->args[k] = NULL;
    }
}


void setupCommand(Command *cmd) {
    if (cmd->inputFile != NULL) {
        int fd_in = open(cmd->inputFile, O_RDONLY);
        if (fd_in == -1) {
            fprintf(stderr, "open read: No such file or directory\n");
            _exit(1);
        }
        if (dup2(fd_in, STDIN_FILENO) == -1) {
            fprintf(stderr, "Error: dup fail\n");
            close(fd_in);
            _exit(1);
        }
        close(fd_in);
    }
    if (cmd->outputFile != NULL) {
        int fd_out = open(cmd->outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd_out == -1) {
            fprintf(stderr, "open read: No such file or directory\n");
            _exit(1);
        }
        if (dup2(fd_out, STDOUT_FILENO) == -1) {
            fprintf(stderr, "Error: dup fail\n");
            close(fd_out);
            _exit(1);
        }
        close(fd_out);
    }
}


int main()
{
    pid_t pid1;
    pid_t pid2;
    pid_t pid3;
    Command command1;
    Command command2;
    Command command3;
    Command* currentCommandWrite;
    bool continueProgram = true;
	int pipeCount;
    bool parseError = false;
    char line[MAX_LINE];
    char* args[MAX_ARGS];
    int token_count;
    int pipeFileDesc[2];


    while(continueProgram)
    {
        init_command_struct(&command1);
        init_command_struct(&command2);
        init_command_struct(&command3);
        currentCommandWrite = &command1;
        pipeCount = 0;
        parseError = false;
        printf("$$ ");

        if(fgets(line, 1024, stdin) != NULL)
        {
            args[0] = strtok(line, STR_SEP);
            token_count = 0;
            while (args[token_count] != NULL)
            {
                ++token_count;
                args[token_count] = strtok(NULL, STR_SEP);
            }

            if (token_count == 0) { // Quick white space check, would make sure less bugs would appear
                continue;
            }

            for (int j = 0; j < token_count; j++) {
                char *current_token = args[j];

                if (strcmp(current_token, "!") == 0) {
                    if (currentCommandWrite->outputFile != NULL) {
                        fprintf(stderr, "Error: Command before pipe cannot have output redirection.\n");
                        parseError = true; break;
                    }
                    currentCommandWrite->args[currentCommandWrite->num_args] = NULL;
                    
                    if (pipeCount == 0) {
                        currentCommandWrite = &command2;
                        pipeCount++;
                    } else if (pipeCount == 1) {
                        if (command2.inputFile != NULL || command2.outputFile != NULL) {
                            fprintf(stderr, "Error: Middle command in pipe cannot have redirections.\n");
                            parseError = true; break;
                        }
                        currentCommandWrite = &command3;
                        pipeCount++;
                    } else {
                        fprintf(stderr, "Error: Too many pipes (max 2).\n");
                        parseError = true; break;
                    }
                    continue;
                }

                if (current_token[0] == '}') {
                    if (currentCommandWrite->inputFile != NULL) { //a quick input check never hurts
                        fprintf(stderr, "Error: Multiple input redirections for a single command part.\n");
                        parseError = true;
                        break;
                    }

                    currentCommandWrite->inputFile = current_token + 1;
                    continue;
                }

                if (current_token[0] == '{') {
                    if (currentCommandWrite->outputFile != NULL) { //a quick input check never hurts
                        fprintf(stderr, "Error: Multiple output redirections for a single command part.\n");
                        parseError = true;
                        break;
                    }

                    currentCommandWrite->outputFile = current_token + 1;
                    continue;
                }

                if (currentCommandWrite->num_args < MAX_ARGS_PER_COMMAND - 1) {
                    currentCommandWrite->args[currentCommandWrite->num_args] = current_token;
                    currentCommandWrite->num_args++;
                } else {
                    fprintf(stderr, "Error: Too many arguments for command.\n");
                    parseError = true;
                    break;
                }
            }
            if ((pipeCount >= 1 && command2.inputFile != NULL) || (pipeCount == 2 && command3.inputFile != NULL)) {
                fprintf(stderr, "Error: Command after pipe cannot have input redirection.\n");
                parseError = true;
            }

            if (parseError) { //just skipping this... the print was already printed at this point
                continue;
            }

            if (pipeCount == 0) { // Single command
                pid1 = fork();
                if(pid1 == 0){
                    setupCommand(&command1);
                    execvp(command1.args[0], command1.args);
                    fprintf(stderr, "execvp: No such file or directory\n");
                    _exit(1);
                } else if (pid1 > 0) {
                    waitpid(pid1, NULL, 0);
                } else {
                    fprintf(stderr, "Error: fork failed\n");
                }
            } 
            else if (pipeCount == 1) { // 2 Commands (1 Pipe)
                if (pipe(pipeFileDesc) == -1) {
                    fprintf(stderr, "Error: pipe failed\n");
                    continue;
                }
                if ((pid1 = fork()) == 0) {
                    close(pipeFileDesc[0]);
                    setupCommand(&command1);
                    if (dup2(pipeFileDesc[1], STDOUT_FILENO) == -1) _exit(1);
                    close(pipeFileDesc[1]);
                    execvp(command1.args[0], command1.args);
                    perror("execvp"); _exit(1);
                }
                if ((pid2 = fork()) == 0) {
                    close(pipeFileDesc[1]);
                    setupCommand(&command2);
                    if (dup2(pipeFileDesc[0], STDIN_FILENO) == -1) _exit(1);
                    close(pipeFileDesc[0]);
                    execvp(command2.args[0], command2.args);
                    perror("execvp"); _exit(1);
                }
                close(pipeFileDesc[0]);
                close(pipeFileDesc[1]);
                waitpid(pid1, NULL, 0);
                waitpid(pid2, NULL, 0);
            }
            else if (pipeCount == 2) { // 3 Commands (2 Pipes)
                int pipeFileDesc2[2];
                if (pipe(pipeFileDesc) == -1 || pipe(pipeFileDesc2) == -1) {
                    fprintf(stderr, "Error: pipe failed\n"); continue;
                }
                
                // Command 1
                if ((pid1 = fork()) == 0) {
                    close(pipeFileDesc[0]); close(pipeFileDesc2[0]); close(pipeFileDesc2[1]);
                    setupCommand(&command1);
                    dup2(pipeFileDesc[1], STDOUT_FILENO);
                    close(pipeFileDesc[1]);
                    execvp(command1.args[0], command1.args);
                    _exit(1);
                }
                
                // Command 2 (Middle)
                if ((pid2 = fork()) == 0) {
                    close(pipeFileDesc[1]); close(pipeFileDesc2[0]);
                    dup2(pipeFileDesc[0], STDIN_FILENO);
                    dup2(pipeFileDesc2[1], STDOUT_FILENO);
                    close(pipeFileDesc[0]); close(pipeFileDesc2[1]);
                    execvp(command2.args[0], command2.args);
                    _exit(1);
                }
                
                // Command 3
                if ((pid3 = fork()) == 0) {
                    close(pipeFileDesc[0]); close(pipeFileDesc[1]); close(pipeFileDesc2[1]);
                    setupCommand(&command3);
                    dup2(pipeFileDesc2[0], STDIN_FILENO);
                    close(pipeFileDesc2[0]);
                    execvp(command3.args[0], command3.args);
                    _exit(1);
                }
                
                close(pipeFileDesc[0]); close(pipeFileDesc[1]);
                close(pipeFileDesc2[0]); close(pipeFileDesc2[1]);
                waitpid(pid1, NULL, 0);
                waitpid(pid2, NULL, 0);
                waitpid(pid3, NULL, 0);
            }
        }

        else
        {
            continueProgram = false;
            printf("\n");
        }
    }
}