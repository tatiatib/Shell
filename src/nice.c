#include "nice.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "execution.h"
extern struct _IO_FILE *stdout;
int cmd_nice(struct tokens *tokens){
	int length = tokens_get_length(tokens);
	if (length == 1){
		int prio = getpriority(PRIO_PROCESS,0);
		if (prio == 125 || prio == 126 || prio == 127){
			fprintf(stderr, "%s\n", strerror(errno));	
			return 1;
		}else
			fprintf(stdout, "%d\n", prio );
	}else{
		char * nice = tokens_get_token(tokens, 1);
		if (strcmp(nice, "-n") == 0){
			if (length <= 2) {
				fprintf(stdout, "%s\n", "nice: option requires an argument -- 'n'");
				return 1;
			}

			char * integer = tokens_get_token(tokens, 2);
			int niceness = atoi(integer);
			char* program[length - 3];
			for (int i = 3; i < length; ++i){
				program[i-3] = tokens_get_token(tokens, i);
			}
			if (strcmp(program[length-3-1], "&") == 0)
				return execute_std_program(program, length - 3, niceness, 1);
			else
				return execute_std_program(program, length - 3, niceness, 0);

		}

		
	}
	
	return 0;
}