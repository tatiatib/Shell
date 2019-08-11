//
// Created by niko on 3/20/18.
//

#ifndef HW1_SHELL_TIMMY_HELPER_H
#define HW1_SHELL_TIMMY_HELPER_H

#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

char in_range(double val, double start, double end);

int index_of(const char *str, const char c);

bool is_allowed_character(const char c, bool is_first);

bool is_int_string(const char *c);

void int_to_string(int val, char buffer[16]);

void reverse(char *str);

void swap(char *a, char *b);

#endif //HW1_SHELL_TIMMY_HELPER_H
