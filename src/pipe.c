#include "pipe.h"
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>


#include "tokenizer.h"
#include "execution.h"


#define BUFFER_SIZE 25
#define READ_END 0
#define WRITE_END 1

/* find full path for file to execute */
extern int check_prgs(char *filename, char **full_path);

struct process {
    char *path;
    char **args;
    int niceness;
    int in;  //
    int out;

    int inp;
    int outp;
};

extern bool shell_is_interactive;
extern int shell_terminal;
extern pid_t shell_pgid;
extern struct _IO_FILE *stdout;


int exec_pipe(struct process* prog, int in, int out, int last){
    if (prog->in != STDIN_FILENO && prog->in != -1) 
        in = prog->in;

    if (prog->out != STDOUT_FILENO && prog->out != -1) 
        out = prog->out;
    

    in = in == -1 ? STDIN_FILENO : in;
    out = out == -1 ? STDOUT_FILENO : out;

    int child_id, status;
    switch (child_id = fork()) {
        case -1:
            perror("fork");
            status = 1;
            break;
        case 0:
            if (in != STDIN_FILENO && in != -1) {
                if (dup2(in, STDIN_FILENO) == -1) {
                    perror("dup2");
                }
                if (close(in) == -1) {
                    perror("close");
                }
            }

            if (out != STDOUT_FILENO && out != -1) {
                if (dup2(out, STDOUT_FILENO) == -1) {
                    perror("dup2");
                }
                if (close(out) == -1) {
                    perror("close");
                }

            }
            if (execv(prog->path, prog->args) == -1) {
                fprintf(stderr, "%s\n", strerror(errno));
            }

            _exit(EXIT_FAILURE);

            break;

        default:
            if (last)
                waitpid(child_id, &status, WUNTRACED);

            break;

    }
    return 0;

}

int execute_piped_command(char *input_line[], int length, int pipe_num, int *piped, int niceness, int background) {
    int process_num = pipe_num + 1;
    struct process *processed[process_num];

    int processLen = 0;
    int lastIndex = 0;
    for (int i = 0; i < length; i++) {
        if (i == length - 1) {
            struct process *prog = parse(&input_line[lastIndex], length - lastIndex);
            prog->inp = STDIN_FILENO;
            prog->outp = STDOUT_FILENO;
            processed[processLen] = prog;
            processLen++;
        } else if (strcmp(input_line[i], "|") == 0) {
            struct process *prog = parse(&input_line[lastIndex], i - lastIndex);
            prog->inp = STDIN_FILENO;
            prog->outp = STDOUT_FILENO;
            processed[processLen] = prog;
            processLen++;
            lastIndex = i + 1;
        }
    }

    int parent_pipe[2];
    int child_pipe[2];
    // int child_pipe[2];
    parent_pipe[0] = parent_pipe[1] = -1;
    child_pipe[0] = child_pipe[1] = -1;


    int status = 0;
    for (int i = 0; i < process_num; i++) {
        if (i == 0) {
            if (pipe(parent_pipe) == -1) {
                perror("error creating pipe");
                return 1;
            }
            status = exec_pipe(processed[i], processed[i]->in, parent_pipe[WRITE_END], 0);
            if (close(parent_pipe[WRITE_END]) == -1)
                perror("closing pipe");
        } else {
            if (i == process_num - 1) {
                if (i % 2 == 1) {
                    status = exec_pipe(processed[i], parent_pipe[READ_END], processed[i]->out, 1);
                    if (close(parent_pipe[READ_END]) == -1)
                        perror("closing pipe");
                } else {
                    status = exec_pipe(processed[i], child_pipe[READ_END], processed[i]->out, 1);
                    if (close(child_pipe[READ_END]) == -1)
                        perror("closing pipe");
                }
            } else if (i % 2 == 1) {
                if (pipe(child_pipe) == -1) {
                    perror("error creating pipe");
                    return 1;
                }
                status = exec_pipe(processed[i], parent_pipe[READ_END], child_pipe[WRITE_END], 0);
                if (close(parent_pipe[READ_END]) == -1)
                    perror("closing pipe");
                if (close(child_pipe[WRITE_END]) == -1)
                    perror("closing pipe");

            } else {
                if (pipe(parent_pipe) == -1) {
                    perror("error creating pipe");
                    return 1;
                }
                status = exec_pipe(processed[i], child_pipe[READ_END], parent_pipe[WRITE_END], 0);
                if (close(child_pipe[READ_END]) == -1)
                    perror("closing pipe");

                if (close(parent_pipe[WRITE_END]) == -1)
                    perror("closing pipe");

            }

        }
    }

    for (int i = 0; i < process_num; i++){
        free(processed[i]->args);
        free(processed[i]->path);
        free(processed[i]);
    }
    return status;
}