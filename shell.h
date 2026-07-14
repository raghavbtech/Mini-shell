#define _POSIX_C_SOURCE 200809L

#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE 1024
#define MAX_ARGS 64

typedef struct {
    char *argv[MAX_ARGS]; /* argument vector; argv[argc] == NULL */
    int   argc;           /* number of arguments                  */
    char *input_file;     /* path for < redirection, or NULL      */
    char *output_file;    /* path for > redirection, or NULL      */
    int   append;         /* reserved: 1 = >> mode                */
} Command;

/* parser.c */
int  parse(char *line, Command *cmd1, Command *cmd2, int *has_pipe);
void free_command(Command *cmd);

/* builtins.c */
int is_builtin(const char *name);
int run_builtin(Command *cmd);

/* executor.c */
void execute(Command *cmd1, Command *cmd2, int has_pipe);

#endif /* SHELL_H */
