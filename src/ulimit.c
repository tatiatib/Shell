#include "ulimit.h"
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


// struct rlimit { 
// rlim_t rlim_cur;  /* Soft limit */
// rlim_t rlim_max;  /* Hard limit (ceiling for rlim_cur) */
// };

extern struct _IO_FILE *stdout;

int resources[15] = {RLIMIT_CORE, RLIMIT_DATA, RLIMIT_NICE, RLIMIT_FSIZE, RLIMIT_SIGPENDING, RLIMIT_MEMLOCK,
                     RLIMIT_RSS, RLIMIT_NOFILE, RLIMIT_MSGQUEUE, RLIMIT_RTPRIO, RLIMIT_STACK,
                     RLIMIT_CPU, RLIMIT_NPROC, RLIMIT_AS, RLIMIT_LOCKS};

/* argument types we can handle */

char *arg_types[16] = {"-c", "-d", "-e", "-f", "-i", "-l", "-m", "-n", "-q", "-r", "-s", "-t", "-u", "-v", "-x", "-a"};
char *output_msgs[15] = {"core file size          (blocks, -c) ",
                         "data seg size           (kbytes, -d) ",
                         "scheduling priority             (-e) ",
                         "file size               (blocks, -f) ",
                         "pending signals                 (-i) ",
                         "max locked memory       (kbytes, -l) ",
                         "max memory size         (kbytes, -m) ",
                         "open files                      (-n) ",
                         "POSIX message queues     (bytes, -q) ",
                         "real-time priority              (-r) ",
                         "stack size               (bytes, -s) ",
                         "cpu time               (seconds, -t) ",
                         "max user processes              (-u) ",
                         "virtual memory          (kbytes, -v) ",
                         "file locks                      (-x) ",
};

static int lookup_args(char *arg) {
    for (int i = 0; i < 16; ++i) {
        if (strcmp(arg_types[i], arg) == 0) {
            return i;
        }
    }

    return -1;

}


static int get_limit_value(int index) {
    struct rlimit rlim;
    int error = getrlimit(resources[index], &rlim);
    if (!error) {
        if ((long long) rlim.rlim_cur == RLIM_INFINITY)
            fprintf(stdout, "%s: %s\n", output_msgs[index], "unlimited");
        else {
            if (rlim.rlim_max != -1 && rlim.rlim_cur > rlim.rlim_max) {
                fprintf(stdout, "%s: %lld\n", output_msgs[index], (long long) rlim.rlim_max);
            } else
                fprintf(stdout, "%s: %lld\n", output_msgs[index], (long long) rlim.rlim_cur);
        }
    } else {
        fprintf(stderr, "%s\n", strerror(errno));
        return 1;
    }

    return 0;
}

static int set_limit_value(int index, char *new_val) {
    int val = atoi(new_val);
    if (val == 0) {
        fprintf(stderr, "%s %s\n %s\n", "-bash: ulimit:", new_val,
                "invalid option ulimit: usage: ulimit [-SHabcdefilmnqrstuvxT] [limit]");
        return 1;
    }

    struct rlimit rlim;
    int error = getrlimit(resources[index], &rlim);
    if (!error) {
        if ((long long) rlim.rlim_max != RLIM_INFINITY && (val > (long long) rlim.rlim_max)) {
            fprintf(stdout, "%s\n", "cannot modify limit: Operation not permitted");
            return 1;
        }
        rlim.rlim_cur = (long long) val;
        error = setrlimit(resources[index], &rlim);
        if (error) {
            fprintf(stderr, "%s\n", strerror(errno));
            return 1;
        }

    } else {
        fprintf(stderr, "%s\n", strerror(errno));
        return 1;
    }


    return 0;
}

int cmd_ulimit(struct tokens *tokens) {
    size_t length = tokens_get_length(tokens);
    int res = -1;
    if (length == 1) {
        return get_limit_value(3);
    }
    if (length == 2 && lookup_args(tokens_get_token(tokens, 1)) < 0) {
        return set_limit_value(3, tokens_get_token(tokens, 1));
    } else {
        for (size_t i = 1; i < length; i++) {
            char *arg = tokens_get_token(tokens, i);
            if (strcmp(arg, "-a") == 0) {
                for (int j = 0; j < 15; j++) {
                    res = get_limit_value(j);
                }

            } else {
                int index = lookup_args(arg);
                if (index >= 0) {
                    if (i + 1 == length || lookup_args(tokens_get_token(tokens, i + 1)) >= 0)
                        res = get_limit_value(index);
                    else {
                        res = set_limit_value(index, tokens_get_token(tokens, i + 1));
                    }
                }
            }
        }
    }

    return res;
}