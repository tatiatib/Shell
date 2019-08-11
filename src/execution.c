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


#include "execution.h"

struct process {
    char *path;
    char **args;
    int niceness;
    int in;
    int out;
    int inp;
    int outp;
};

pid_t child_id;
static int back;
extern struct termios shell_tmodes;
extern int shell_terminal;
extern pid_t shell_pgid;
extern struct _IO_FILE *stdout;

/* find full path for file to execute */
extern int check_prgs(char *filename, char **full_path);


static void process_fds(struct process **pr, char *redirect_sign, char *filename) {
    int fd;
    if (strcmp(redirect_sign, "<") == 0) {
        fd = open(filename, O_RDONLY);
        if (fd == -1)
            perror("file not found");
        (*pr)->in = fd;
    }
    if (strcmp(redirect_sign, ">") == 0) {
        fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (fd == -1)
            perror("file not found");
        (*pr)->out = fd;
    }

    if (strcmp(redirect_sign, ">>") == 0) {
        fd = open(filename, O_RDWR | O_CREAT | O_APPEND, 0666);
        if (fd == -1)
            perror("file not found");
        (*pr)->out = fd;
    }

}

static int search(struct process **pr, char *input_line[], int length) {
    int index = -1;
    for (int i = 0; i < length; i++) {
        if (strcmp(input_line[i], ">") == 0 || strcmp(input_line[i], "<") == 0 || strcmp(input_line[i], ">>") == 0) {
            char *filename = input_line[i + 1];
            process_fds(pr, input_line[i], filename);
            if (index == -1) index = i;
        }
    }
    if (index == -1)
        return length;
    else
        return index;
}


void signals() {
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);
}

/* parsing 	input line, finding full path for file to execute program and create new process variable */
struct process *parse(char *input_line[], int length) {
    if (strcmp(input_line[0], "echo") == 0) {
        input_line[0] = strdup("/bin/echo");
    }
    struct process *new_process = (struct process *) malloc(sizeof(struct process));
    char *path;
    char **args;
    new_process->in = STDIN_FILENO;
    new_process->out = STDOUT_FILENO;

    args = (char **) malloc((length + 1) * sizeof(char *));
    args[0] = strrchr(input_line[0], '/');

    if (args[0] != NULL) {
        args[0]++;
        path = input_line[0];
    } else {
        int prg = check_prgs(input_line[0], &path);
        if (prg != 0) {

            path = input_line[0];
        }

        args[0] = input_line[0];
    }
    if (length > 1) {
        int size = search(&new_process, input_line, length);
        if (new_process->in == -1 || new_process->out == -1) return NULL;

        memcpy(args + 1, input_line + 1, (size - 1) * sizeof(char *));
        args[size] = NULL;
    } else
        args[length] = NULL;


    new_process->path = path;
    new_process->args = args;

    return new_process;

}

int status;

int execute_std_program(char *input_line[], int length, int niceness, int background) {

    struct process *prog = parse(input_line, length);
    back = background;
    if (prog == NULL) return 1;
    prog->niceness = niceness;


    switch (child_id = fork()) {
        case -1:
            perror("fork");
            status = 1;
            break;

        case 0:
            signals();
            if (niceness != 0) {
                if (setpriority(PRIO_PROCESS, 0, niceness) == 1)
                    perror("Can't set priority");
            }

            dup2(prog->in, STDIN_FILENO);
            if (prog->in != STDIN_FILENO && close(prog->in) == -1) {
                perror("close");
            }
            dup2(prog->out, STDOUT_FILENO);
            if (prog->out != STDOUT_FILENO && close(prog->out) == -1) {
                perror("close");
            }

            signals();
            if (execv(prog->path, prog->args) == -1) {
                fprintf(stderr, "%s\n", strerror(errno));
            }

            _exit(EXIT_FAILURE);

            break;

        default:
            if (!back) {
                setpgid(child_id, child_id);
                tcsetpgrp(STDIN_FILENO, child_id);
                waitpid(child_id, &status, WUNTRACED);
                tcsetpgrp(STDIN_FILENO, shell_pgid);

            } else {
                fprintf(stdout, "[%d]\n", child_id);

            }

    }

    free(prog->args);
    free(prog->path);
    free(prog);


    return status;
}