#ifndef _KILL_H
#define _KILL_H
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "tokenizer.h"

/* killing program */
/* KILL -XXX PID   -XXX is INT, QUIT or other human readable signals */
int cmd_kill(struct tokens *tokens);

#endif