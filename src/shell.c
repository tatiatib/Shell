#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include "pipe.h"
#include "tokenizer.h"
#include "ulimit.h"
#include "echo.h"
#include "nice.h"
#include "execution.h"
#include "var.h"
#include "export.h"
#include "kill.h"

// #include "pipe.h"

/* Convenience macro to silence compiler warnings about unused function parameters. */

/* Whether the shell is connected to an actual terminal or not. */
bool shell_is_interactive;


/* File descriptor for the shell input */
int shell_terminal;

/* Terminal mode settings for the shell */
struct termios shell_tmodes;

/* Process group id for the shell */
pid_t shell_pgid;


int cmd_exit(struct tokens *tokens);

int cmd_help(struct tokens *tokens);

int cmd_type(struct tokens *tokens);

int cmd_cd(struct tokens *tokens);

int cmd_pwd(struct tokens *tokens);

static int status;

void destroy_map();

int processToken(DictionaryNode *map, char *token, char *result, int pointer);


static void processArgument(struct tokens *tokens, size_t arg_index, DictionaryNode *map);

static int append(char *from, char *to, int pointer);

void process_line(char *line, int *line_num);

int process_command(char *command);

int get_exit_status(struct tokens *pTokens);

char *get_status();

/* Built-in command functions take token array (see parse.h) and return int */
typedef int cmd_fun_t(struct tokens *tokens);

/* Built-in command struct and lookup table */
typedef struct fun_desc {
    cmd_fun_t *fun;
    char *cmd;
    char *doc;
} fun_desc_t;

fun_desc_t cmd_table[] = {
        {cmd_help,   "?",      "show this help menu"},
        {cmd_exit,   "exit",   "exit the command shell"},
        {cmd_type,   "type",   "Display information about command type"},
        {cmd_echo,   "echo",   " Display a line of text"},
        {cmd_ulimit, "ulimit", "Get and set user limits"},
        {cmd_cd,     "cd",     "change directory"},
        {cmd_pwd,    "pwd",    "print directory name"},
        {cmd_nice,   "nice",   "Run a program with modified scheduling priority"},
        {cmd_export, "export", "Export a variable"},
        {cmd_kill,   "kill",   "Send a signal to a process"}
};

static DictionaryNode *var_map;

int cmd_cd(unused struct tokens *tokens) {
    if (!strcmp(tokens_get_token(tokens, 0), "cd")) {
        int res = !chdir(tokens_get_token(tokens, 1));
        if (res) {
            printf("\\%s\n", tokens_get_token(tokens, 1));
        } else {
            printf("%s: chdir failed\n", tokens_get_token(tokens, 1));
        }

    }
    return 1;
}

int cmd_pwd(unused struct tokens *tokens) {
    char buf[200];
    size_t size = 200;
    if (!strcmp(tokens_get_token(tokens, 0), "pwd")) {
        char *res = getcwd(buf, size);
        printf("%s\n", res);
    }
    return 1;
}


/*Checking file represantion of command in file tree */
int check_prgs(char *filename, char **full_path) {
    char *path = getenv("PATH");
    struct tokens *path_tokens = tokenize(path, ':');
    size_t length = tokens_get_length(path_tokens);

    int res = 1;
    for (size_t i = 0; i < length; i++) {
        char *token = malloc(strlen(tokens_get_token(path_tokens, i)) + strlen(filename) + 2);
        strcpy(token, tokens_get_token(path_tokens, i));
        strcat(token, "/");
        char *cur_path = strcat(token, filename);
        struct stat s;
        if (stat(cur_path, &s) == 0 && S_ISREG(s.st_mode)) {
            *full_path = malloc(strlen(cur_path) + 1);
            strcpy(*full_path, cur_path);
            res = 0;
            free(token);
            break;
        }
        free(token);

    }

    tokens_destroy(path_tokens);
    return res;
}

/*Checking command type in shell builtins */
int check_builtins(char *token) {
    for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++) {
        if (strcmp(cmd_table[i].cmd, token) == 0) {
            fprintf(stdout, "%s %s\n", token, "is a shell builtin");
            return 0;
        }
    }
    return 1;
}

/* Prints information about command type passed as argument*/
int cmd_type(unused struct tokens *tokens) {
    size_t length = tokens_get_length(tokens);
    char *full_path;
    if (length <= 1) return 0;
    for (size_t i = 1; i < length; i++) {
        char *token = tokens_get_token(tokens, i);
        int blt = 0;
        int prg = 0;
        if (strcmp(token, "-a") == 0) {  //request for all entries for the type
            i++;
            if (i == length) return 1;
            token = tokens_get_token(tokens, i);
            blt = check_builtins(token);
            prg = check_prgs(token, &full_path);
            if (prg == 0) {
                fprintf(stdout, "%s is %s\n", token, full_path);

            }
            free(full_path);

        } else {
            blt = check_builtins(token);
            if (blt) {
                prg = check_prgs(token, &full_path);
                if (prg == 0) {
                    fprintf(stdout, "%s is %s\n", token, full_path);

                }
                free(full_path);
            }
        }
        free(full_path);

        if (blt && prg) {  //Nothing found
            fprintf(stdout, "%s %s %s\n", "bash: type:", token, " not found");
            return 1;
        }

    }
    return 0;
}

/* Prints a helpful description for the given command */
int cmd_help(unused struct tokens *tokens) {
    for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
        printf("%s - %s\n", cmd_table[i].cmd, cmd_table[i].doc);
    return 1;
}

/* Exits this shell */
int cmd_exit(unused struct tokens *tokens) {
    int exit_status = get_exit_status(tokens);

    destroy_map();
    tokens_destroy(tokens);
    exit(exit_status);
}

int get_exit_status(struct tokens *pTokens) {
    int l = tokens_get_length(pTokens);
    if (l == 1) return 0;
    char *command = tokens_get_token(pTokens, 1);
    if (!is_int_string(command)) return 0;
    return atoi(command);
}

void destroy_map() {
    if (var_map == NULL)
        return;
    dictionary_dispose(var_map);
    free(var_map);
    var_map = NULL;
}

/* Looks up the built-in command, if it exists. */
int lookup(char cmd[]) {
    for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
        if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0))
            return i;
    return -1;
}


/* Intialization procedures for this shell */
void init_shell() {
    /* Our shell is connected to standard input. */
    shell_terminal = STDIN_FILENO;

    /* Check if we are running interactively */
    shell_is_interactive = isatty(shell_terminal);

    if (shell_is_interactive) {

        /* If the shell is not currently in the foreground, we must pause the shell until it becomes a
         * foreground process. We use SIGTTIN to pause the shell. When the shell gets moved to the
         * foreground, we'll receive a SIGCONT. */
        while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp()))
            kill(-shell_pgid, SIGTTIN);

        /* Saves the shell's process id */
        shell_pgid = getpid();

        /* Take control of the terminal */
        tcsetpgrp(shell_terminal, shell_pgid);

        /* Save the current termios to a variable, so it can be restored later. */
        tcgetattr(shell_terminal, &shell_tmodes);
    }
}


void handle_signals() {
    signal(SIGINT, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
}


int parseCondition(struct tokens *tokens, int start, int end) {

    int fundex = lookup(tokens_get_token(tokens, 0));
    if (fundex >= 0) {
        cmd_table[fundex].fun(tokens);
    } else if (is_variable_assignment(tokens)) {
        assign_variable(tokens, var_map);
    } else {
        int piped[end - start];
        int index = 0;
        // Just for testing reasons, this should't be here
        char *program_types[end - start];
        int j = 0;
        for (size_t i = start; i < end; i++) {
            char *token = tokens_get_token(tokens, i);
            if (strcmp(token, "|") == 0) {
                piped[index] = i;
                index++;
            }
            program_types[j] = tokens_get_token(tokens, i);
            j++;
        }

        if (index != 0) {
            // printf("%s\n", "piped");
            status = execute_piped_command(program_types, end - start, index, piped, 0, 0);
        } else {
            if (strcmp(program_types[end - start - 1], "&") == 0)
                status = execute_std_program(program_types, end - start - 1, 0, 1);
            else
                status = execute_std_program(program_types, end - start, 0, 0);
        }
        printf("program ended with exit code %d\n", status);
        /* REPLACE this to run commands as programs. */
        // fprintf(stdout, "This shell doesn't know how to run programs.\n");
    }

    return status;
}


int finedNextEndOperatorIndex(struct tokens *tokens, int currentOperatorIndex) {
    currentOperatorIndex++;
    for (int i = currentOperatorIndex; i < tokens_get_length(tokens); i++) {
        char *token = tokens_get_token(tokens, i);
        if (strcmp(token, "&&") == 0)
            return i;
    }
    return 0;
}

int finedNextOrOperatorIndex(struct tokens *tokens, int currentOperatorIndex) {
    currentOperatorIndex++;
    for (int i = currentOperatorIndex; i < tokens_get_length(tokens); i++) {
        char *token = tokens_get_token(tokens, i);
        if (strcmp(token, "||") == 0)
            return i;
    }
    return 0;
}

int parseLine(struct tokens *tokens) {
    int lastOperatorIndex = 0;
    for (int i = 0; i < tokens_get_length(tokens); i++) {

        char *token = tokens_get_token(tokens, i);
        if (i == tokens_get_length(tokens) - 1) {
            parseCondition(tokens, lastOperatorIndex, tokens_get_length(tokens));
        } else if (strcmp(token, "||") == 0 || strcmp(token, "&&") == 0) {
            char *operator = "";
            if (strcmp(token, "||")) {
                operator = "||";
            } else {
                operator = "&&";
            }
            int res = parseCondition(tokens, lastOperatorIndex, i);
            if (res != 0 && strcmp(operator, "&&")) {
                int finedNextOrOperator = finedNextOrOperatorIndex(tokens, i);
                if (finedNextOrOperator != 0) {
                    i = finedNextOrOperator;
                } else {
                    break;
                }
            }
            if (res == 0 && strcmp(operator, "||")) {
                int nextEndOperatorIndex = finedNextEndOperatorIndex(tokens, i);
                if (nextEndOperatorIndex != 0) {
                    i = nextEndOperatorIndex;
                } else {
                    break;
                }
            }
            lastOperatorIndex = i + 1;
        }
    }
    return status;
}


int main(unused int argc, unused char *argv[]) {
//    id = getpid();
    status = -1;
    init_shell();
    handle_signals();
    var_map = malloc(sizeof(DictionaryNode));
    dictionary_new(var_map, free);
    if (argc > 1 && !strcmp("-c", argv[1])) {
        shell_is_interactive = false;
        if (argc < 3) {
            printf("Error: -c: option requires an argument.\n");
            return 1;
        }
        return process_command(argv[2]);

    }
    static char line[4096];
    int line_num = 0;
    /* Please only print shell prompts when standard input is not a tty */
    if (shell_is_interactive)
        fprintf(stdout, "%d: ", line_num);


    while (fgets(line, 4096, stdin)) {
        process_line(line, &line_num);
    }
    destroy_map();
    return 0;
}

int process_command(char *command) {
    struct tokens *tokens = tokenize(command, ';');
    size_t count = tokens_get_length(tokens);
    for (size_t i = 0; i < count; ++i) {
        process_line(tokens_get_token(tokens, i), (int *) &i);
    }
    tokens_destroy(tokens);
    return status == 0 ? 0 : 1;
}

void process_line(char *line, int *line_num) {
    /* Split our line into words. */
    struct tokens *tokens = tokenize(line, ' ');

//    pid_t id = getpid();
    size_t length = tokens_get_length(tokens);
    for (size_t i = 0; i < length; ++i) {
        processArgument(tokens, i, var_map);
    }

    status = parseLine(tokens);

    if (shell_is_interactive)
        /* Please only print shell prompts when standard input is not a tty */
        fprintf(stdout, "%d: ", ++line_num[0]);

    /* Clean up memory */
//    tokens_destroy(tokens);

}

DictionaryNode *get_var_map() {
    return var_map;
}


static void processArgument(struct tokens *tokens, size_t arg_index, DictionaryNode *map) {
    if (is_squote_token(tokens, arg_index)) return;
    char *argument = tokens_get_token(tokens, arg_index);
    struct tokens *var_tokens = tokenize(argument, '$');
    size_t l = tokens_get_length(var_tokens);
    int pointer = 0;
    char processed[6200];
    memset(processed, 0, 6200 * sizeof(char));
    pointer = (argument[0] == '$' ?
               processToken(map, tokens_get_token(var_tokens, 0), processed, pointer)
                                  : append(tokens_get_token(var_tokens, 0), processed, 0));
    for (size_t i = 1; i < l; ++i) {
        pointer = processToken(map, tokens_get_token(var_tokens, i), processed, pointer);
    }
    token_replace(tokens, arg_index, processed);
    tokens_destroy(var_tokens);
}

static int append(char *from, char *to, int pointer) {
    size_t l = strlen(from);
    strcpy(to + pointer, from);
    return pointer + (int) l;
}

int processToken(DictionaryNode *map, char *token, char *result, int pointer) {
    size_t l = strlen(token);
    char key[l + 1];
    memset(key, 0, (l + 1) * sizeof(char));
    size_t keysize = 0;
    if (token[0] == '?' || !is_allowed_character(token[0], true)) {
        keysize = 1;
    } else
        for (int i = 0; i < l && is_allowed_character(token[i], false); ++i) {
            keysize++;
        }

    memcpy(key, token, keysize * sizeof(char));
    char *val = token[0] == '?' ? get_status() : dictionary_get(map, key);
    if (val == NULL) val = getenv(key);
    if (val == NULL) val = "";
    pointer = append(val, result, pointer);
    if (token[0] == '?') free(val);
    return append(token + keysize, result, pointer);
}

char *get_status() {
    char *value = malloc(sizeof(char) * 16);
    memset(value, 0, 16 * sizeof(char));
    int_to_string(status, value);
    return value;
}