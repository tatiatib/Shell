#ifndef HW1_SHELL_TIMMY_VAR_H
#define HW1_SHELL_TIMMY_VAR_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "helper.h"
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include "tokenizer.h"
#include "dictionary.h"

/*
 * Function:  is_variable_assignment
 * ---------------------
 * Returns whether the given command is a variable assignment
 */
char is_variable_assignment(struct tokens *pTokens);

/*
 * Function: assign_variable
 * ---------------------
 *
 *
 * */
void assign_variable(struct tokens *pTokens, DictionaryNode *dictionary);

void set_variable(const char *command, DictionaryNode *dictionary);

#endif