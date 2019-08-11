DIR=./src
SRCS= $(wildcard $(DIR)/*.c)
BUILD=./build
EXECUTABLES=build/shell
EXECUTABLES=shell 
CC=gcc
CFLAGS=-g -Wall -std=gnu99
LDFLAGS=

OBJS= $(patsubst $(DIR)/%.c,$(BUILD)/%.o,$(SRCS))
#$(SRCS:%.c=$(BUILD)/%.o)

 $(EXECUTABLES): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@

all:
	$(EXECUTABLES)

$(OBJS): $(BUILD)/%.o : $(DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(EXECUTABLES) $(OBJS)
