#ifndef _ECHO_H
#define _ECHO_H

#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "tokenizer.h"
#include "shell.h"

//handles echo type commands
int cmd_echo(struct tokens *tokens);


#endif 