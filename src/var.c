
#include <ctype.h>
#include "var.h"


bool valid_command(char *command);

void generate_value(const char *command, size_t eq_i, size_t l, char buffer[4096], const DictionaryNode *dictionary);


char is_variable_assignment(struct tokens *pTokens) {
    if (tokens_get_length(pTokens) != 1)return 0;
    char *command = tokens_get_token(pTokens, 0);
    int index = index_of(command, '=');
    return index > 0 && valid_command(command);
}

bool valid_command(char *command) {
    if (command[0] == '=' || in_range(command[0], '0', '9') || command[0] == '$') return false;
    size_t eq_index = (size_t) index_of(command, '=');
    for (int i = 1; i < eq_index; ++i) {
        char lo = (char) tolower(command[i]);
        if (!in_range(lo, 'a', 'z') && !in_range(lo, '0', '9')) return false;
    }
    return true;
}


void assign_variable(struct tokens *pTokens, DictionaryNode *dictionary) {
    char *command = tokens_get_token(pTokens, 0);
    set_variable(command, dictionary);
}

void set_variable(const char *command, DictionaryNode *dictionary) {
    size_t l = strlen(command);
    char varname[512];
    char buffer[4096];
    memset(buffer, 0, sizeof(char) * 4096);
    memset(varname, 0, sizeof(char) * 512);
    size_t i = (size_t) index_of(command, '=');
    strncpy(varname, command, i);
    generate_value(command, i, l, buffer, dictionary);

    char *value = strdup(buffer);
    if (getenv(varname)) {
        setenv(varname, value, 1);
    }
    dictionary_put(dictionary, varname, value);

}

void generate_value(const char *command, size_t eq_i, size_t l, char buffer[4096], const DictionaryNode *dictionary) {
    for (int i = (int) eq_i + 1, j = 0; i < l;) {
        if (command[i] == '$') {
            int env_l = 0;
            for (int env_i = i + 1; env_i < l; ++env_i) {
                if (!is_allowed_character(command[env_i], env_i == i + 1)) {
                    if (env_i == i + 1) i++;
                    break;
                }
                env_l++;
            }
            if (env_l != 0) {
                char env_var[env_l + 1];
                env_var[env_l] = 0;
                strncpy(env_var, command + i + 1, (size_t) env_l);
                char *value = dictionary_get(dictionary, env_var);
                if (value == NULL) value = getenv(env_var);
                i += env_l + 1;
                if (value == NULL) {
                    continue;
                }
                strcpy(buffer + j, value);
                j += strlen(value);
            } else i++;
        } else {
            buffer[j] = command[i];
            j++;
            i++;
        }
    }
}

