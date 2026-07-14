#define _POSIX_C_SOURCE 200809L

#include "shell.h"

static const char *BUILTIN_NAMES[] = {"cd", "pwd", "help", "exit", NULL};

int is_builtin(const char *name) {
  for (int i = 0; BUILTIN_NAMES[i]; i++)
    if (strcmp(name, BUILTIN_NAMES[i]) == 0)
      return 1;
  return 0;
}

static int builtin_cd(Command *cmd) {
  const char *dir = (cmd->argc > 1) ? cmd->argv[1] : getenv("HOME");
  if (!dir) {
    fprintf(stderr, "cd: HOME not set\n");
    return 1;
  }
  if (chdir(dir) < 0) {
    perror("cd");
    return 1;
  }
  return 0;
}

static int builtin_pwd(void) {
  char cwd[MAX_LINE];
  if (!getcwd(cwd, sizeof(cwd))) {
    perror("pwd");
    return 1;
  }
  printf("%s\n", cwd);
  return 0;
}

static int builtin_help(void) {
  printf("Mini Shell  --  built-in commands:\n");
  printf("  cd [dir]  change directory (defaults to $HOME)\n");
  printf("  pwd       print working directory\n");
  printf("  help      show this message\n");
  printf("  exit      exit the shell\n");
  return 0;
}

int run_builtin(Command *cmd) {
  if (strcmp(cmd->argv[0], "cd") == 0)
    return builtin_cd(cmd);
  if (strcmp(cmd->argv[0], "pwd") == 0)
    return builtin_pwd();
  if (strcmp(cmd->argv[0], "help") == 0)
    return builtin_help();
  if (strcmp(cmd->argv[0], "exit") == 0) {
    printf("Goodbye.\n");
    exit(0);
  }
  return 1;
}
