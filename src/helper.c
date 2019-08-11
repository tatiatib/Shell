//
// Created by niko on 3/20/18.
//

#include "helper.h"

void to_str(int val, char *buffer);

char in_range(double val, double start, double end) {
    return start <= val && val <= end;
}

bool is_allowed_character(const char c, bool is_first) {
    char lower = (char) tolower(c);
    int is_underscore = lower == '_',
            is_alphabetic = in_range(lower, 'a', 'z'), is_digit_allowed = (!is_first && in_range(lower, '0', '9'));
    return is_underscore || is_alphabetic || is_digit_allowed;
}

int index_of(const char *str, const char c) {
    size_t l = strlen(str);
    for (int i = 0; i < l; ++i) {
        if (str[i] == c) return i;
    }
    return -1;
}

bool is_int_string(const char *c) {
    int l = strlen(c);
    for (int i = 0; i < l; ++i) {
        if (!in_range(c[i], '0', '9')) {
            if (i == 0 && c[i] == '-') continue;
            return false;
        }

    }
    return true;
}


void int_to_string(int val, char *buffer) {
    to_str(abs(val), buffer);
    size_t l = strlen(buffer);
    if (l > 1 && buffer[l - 1] == '0')buffer[l - 1] = 0;
    if (val < 0) {
        l = strlen(buffer);
        buffer[l] = '-';
    }
    reverse(buffer);
}

void to_str(int val, char *buffer) {


    *buffer = (char) (val % 10 + '0');
    if (val == 0)return;
    to_str(val / 10, buffer + 1);
}

void reverse(char *str) {
    size_t l = strlen(str);
    for (int i = 0, j = (int) l - 1; i <= j; ++i, j--) {
        swap(str + i, str + j);
    }
}

void swap(char *a, char *b) {
    char tmp = *a;
    *a = *b;
    *b = tmp;
}

