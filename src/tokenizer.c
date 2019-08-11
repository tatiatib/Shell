
#include "tokenizer.h"

struct tokens {
    size_t tokens_length;
    char **tokens;
    size_t buffers_length;
    char **buffers;
    bool *squotes;
    size_t squotes_length;
};

static void *vector_push(char ***pointer, size_t *size, void *elem) {
    *pointer = (char **) realloc(*pointer, sizeof(char *) * (*size + 1));
    (*pointer)[*size] = elem;
    *size += 1;
    return elem;
}

static void *copy_word(char *source, size_t n) {
    source[n] = '\0';
    char *word = (char *) malloc(n + 1);
    strncpy(word, source, n + 1);
    return word;
}


void array_grow(bool **array, size_t length, size_t *loglen);

struct tokens *tokenize(const char *line, char delimiter) {
    if (line == NULL) {
        return NULL;
    }

    static char token[4096];
    size_t n = 0, n_max = 4096;
    struct tokens *tokens;
    size_t line_length = strlen(line);

    tokens = (struct tokens *) malloc(sizeof(struct tokens));
    tokens->tokens_length = 0;
    tokens->tokens = NULL;
    tokens->buffers_length = 0;
    tokens->buffers = NULL;
    tokens->squotes = malloc(sizeof(bool) * 8);
    tokens->squotes_length = 8;
    memset(tokens->squotes, false, 8 * sizeof(bool));
    const int MODE_NORMAL = 0,
            MODE_SQUOTE = 1,
            MODE_DQUOTE = 2;
    int mode = MODE_NORMAL;

    for (unsigned int i = 0; i < line_length; i++) {
        char c = line[i];
        if (mode == MODE_NORMAL) {
            if (c == '\'') {
                mode = MODE_SQUOTE;
            } else if (c == '"') {
                mode = MODE_DQUOTE;
            } else if (c == '\\') {
                if (i + 1 < line_length) {
                    token[n++] = line[++i];
                }
            } else if (c == delimiter || (isspace(c) && c != ' ')) {
                if (n > 0) {
                    void *word = copy_word(token, n);
                    vector_push(&tokens->tokens, &tokens->tokens_length, word);
                    n = 0;
                }
            } else {
                token[n++] = c;
            }
        } else if (mode == MODE_SQUOTE) {
            if (c == '\'') {
                mode = MODE_NORMAL;
                array_grow(&tokens->squotes, tokens->tokens_length, &tokens->squotes_length);
                tokens->squotes[tokens->tokens_length] = true;
            } else if (c == '\\') {
                if (i + 1 < line_length) {
                    token[n++] = line[++i];
                }
            } else {
                token[n++] = c;
            }
        } else if (mode == MODE_DQUOTE) {
            if (c == '"') {
                mode = MODE_NORMAL;
            } else if (c == '\\') {
                if (i + 1 < line_length) {
                    token[n++] = line[++i];
                }
            } else {
                token[n++] = c;
            }
        }
        if (n + 1 >= n_max) abort();
    }

    if (n > 0) {
        void *word = copy_word(token, n);
        vector_push(&tokens->tokens, &tokens->tokens_length, word);
        n = 0;
    }
    return tokens;
}

void array_grow(bool **array, size_t length, size_t *loglen) {
    if (*loglen <= length - 1) {
        array[0] =
                realloc(array[0], 2 * (*loglen) * sizeof(char));
        *loglen *= 2;
    }
}


size_t tokens_get_length(struct tokens *tokens) {
    if (tokens == NULL) {
        return 0;
    } else {
        return tokens->tokens_length;
    }
}

char *tokens_get_token(struct tokens *tokens, size_t n) {
    if (tokens == NULL || n >= tokens->tokens_length) {
        return NULL;
    } else {
        return tokens->tokens[n];
    }
}

void tokens_destroy(struct tokens *tokens) {
    if (tokens == NULL) {
        return;
    }
    for (int i = 0; i < tokens->tokens_length; i++) {
        free(tokens->tokens[i]);
    }
    for (int i = 0; i < tokens->buffers_length; i++) {
        free(tokens->buffers[i]);
    }
    if (tokens->tokens) {
        free(tokens->tokens);
    }
    free(tokens->squotes);
    free(tokens);
}

void token_replace(struct tokens *t, size_t i, char *argument) {
    if (!in_range(i, 0, t->tokens_length - 1))return;
    char *dup = strdup(argument);
    free(t->tokens[i]);
    t->tokens[i] = dup;

}

bool is_squote_token(struct tokens *t, size_t index) {
    return t->squotes[index];
}
