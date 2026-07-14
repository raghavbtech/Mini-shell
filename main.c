#define _POSIX_C_SOURCE 200809L

#include "shell.h"

int main(void) {
  char line[MAX_LINE];
  Command cmd1, cmd2;
  int has_pipe;

  while (1) {
    printf("shell> ");
    fflush(stdout);

    if (!fgets(line, sizeof(line), stdin)) {
      printf("\n");
      break; /* EOF — Ctrl+D */
    }

    line[strcspn(line, "\n")] = '\0'; /* strip trailing newline */

    if (line[0] == '\0')
      continue;

    has_pipe = 0;
    if (parse(line, &cmd1, &cmd2, &has_pipe) < 0)
      continue; /* parse already cleaned up */

    if (cmd1.argc > 0) {
      if (is_builtin(cmd1.argv[0]))
        run_builtin(&cmd1);
      else
        execute(&cmd1, &cmd2, has_pipe);
    }

    free_command(&cmd1);
    free_command(&cmd2); /* safe even when unused — all fields are NULL */
  }

  return 0;
}
