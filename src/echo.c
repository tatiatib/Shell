#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "echo.h"

extern struct _IO_FILE *stdout;


int cmd_echo(struct tokens *tokens) {
    size_t length = tokens_get_length(tokens);
    if (length <= 1) return 1;
    for (int i = 1; i < length; ++i) {
        char *argument = tokens_get_token(tokens, i);
        printf("%s ", argument);
    }
    printf("\n");
    return 0;
}

