//
// Created by niko on 3/22/18.
//

#include <ctype.h>
#include "export.h"
#include "var.h"


bool is_assigning(char *name);

bool is_valid(char *command);

int cmd_export(struct tokens *tokens) {
    DictionaryNode *map = get_var_map();
    if (!strcmp(tokens_get_token(tokens, 0), "export")) {
        size_t token_count = tokens_get_length(tokens);

        for (int i = 1; i < token_count; ++i) {
            char *var_name = tokens_get_token(tokens, 1);
            if (!is_valid(var_name)) {
                printf("export: '%s': not a valid identifier\n", var_name);
                continue;
            }
            if (is_assigning(var_name)) {
                set_variable(var_name, map);
                var_name[index_of(var_name, '=')] = 0;
            }
            char *var_val = dictionary_get(map, var_name);
            if (var_val) {
                setenv(var_name, var_val, 1);
            }
        }
    }
    return 1;
}

bool is_valid(char *command) {

    if (command[0] == '=' || in_range(command[0], '0', '9')) return false;
    int eq_index = index_of(command, '=');
    int start = eq_index > 0 ? 1 : 0, end = eq_index > 0 ? eq_index : (int) strlen(command);
    for (int i = start; i < end; ++i) {
        char lo = (char) tolower(command[i]);
        if (!in_range(lo, 'a', 'z') && !in_range(lo, '0', '9')) return false;
    }
    return true;
}

bool is_assigning(char *command) {
    int eq_index = index_of(command, '=');
    return eq_index > 0;
}
