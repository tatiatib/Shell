#pragma once

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "helper.h"

/* A struct that represents a list of words. */
struct tokens;

/* Turn a string into a list of words. */
struct tokens *tokenize(const char *line, char delimiter);

/* How many words are there? */
size_t tokens_get_length(struct tokens *tokens);

/* Get me the Nth word (zero-indexed) */
char *tokens_get_token(struct tokens *tokens, size_t n);

/* Free the memory */
void tokens_destroy(struct tokens *tokens);

void token_replace(struct tokens *t, size_t i, char *argument);

/* Returns whether the given token was in single quotes when tokenized*/
bool is_squote_token(struct tokens *t, size_t index);