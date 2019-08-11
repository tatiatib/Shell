#include "kill.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>

#define size 8
extern pid_t shell_pgid;

int cmd_kill(struct tokens *tokens){
	int length = tokens_get_length(tokens);
	if (length < 3) return 1;

	int signal = atoi(tokens_get_token(tokens, 1) + 1);
	pid_t id = atoi(tokens_get_token(tokens, 2));

	if (kill(id, signal) == -1){
		fprintf(stderr, "%s\n", strerror(errno));
		return 1;
	}

	return 0;
	
}
