#ifndef _EXECUTION_H
#define _EXECUTION_H

#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/* struct describes path (where file is located),
	arguments and other stuff */
struct process;
/* execute standard type of programs, such as:
	1. ls
	2. gcc wc.c
	3. cat shell.c
	....
	*/

/* args :  command line as input_line
			length - length  of comman line
			niceness - integer of niceness
			background - 0 for foregroung process, 1 for background process*/
int execute_std_program(char *input_line[], int length, int niceness, int background);

struct process *parse(char *input_line[], int length);

#endif