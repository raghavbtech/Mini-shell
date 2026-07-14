#define _POSIX_C_SOURCE 200809L

#include "shell.h"

static void init_command(Command *cmd) {
  cmd->argc = 0;
  cmd->input_file = NULL;
  cmd->output_file = NULL;
  cmd->append = 0;
  memset(cmd->argv, 0, sizeof(cmd->argv));
}

void free_command(Command *cmd) {
  for (int i = 0; i < cmd->argc; i++) {
    free(cmd->argv[i]);
    cmd->argv[i] = NULL;
  }
  free(cmd->input_file);
  free(cmd->output_file);
  cmd->input_file = NULL;
  cmd->output_file = NULL;
  cmd->argc = 0;
}

/*
 * Populate one Command from tokens[start..end).
 * Each accepted token is strdup'd so the command owns its strings.
 */
static int fill_command(char **tokens, int start, int end, Command *cmd) {
  for (int i = start; i < end; i++) {
    if (strcmp(tokens[i], "<") == 0) {
      if (i + 1 >= end) {
        fprintf(stderr, "shell: syntax error: missing input file\n");
        return -1;
      }
      free(cmd->input_file);
      cmd->input_file = strdup(tokens[++i]);
      if (!cmd->input_file) {
        perror("strdup");
        return -1;
      }

    } else if (strcmp(tokens[i], ">") == 0) {
      if (i + 1 >= end) {
        fprintf(stderr, "shell: syntax error: missing output file\n");
        return -1;
      }
      free(cmd->output_file);
      cmd->output_file = strdup(tokens[++i]);
      if (!cmd->output_file) {
        perror("strdup");
        return -1;
      }

    } else {
      if (cmd->argc >= MAX_ARGS - 1) {
        fprintf(stderr, "shell: too many arguments\n");
        return -1;
      }
      cmd->argv[cmd->argc] = strdup(tokens[i]);
      if (!cmd->argv[cmd->argc]) {
        perror("strdup");
        return -1;
      }
      cmd->argc++;
    }
  }
  cmd->argv[cmd->argc] = NULL;
  return 0;
}

/*
 * Parse a raw input line into up to two Commands.
 * Supports arguments, < and > redirections, and one pipe (|).
 * Sets *has_pipe to 1 when a pipe is found; cmd2 is valid only then.
 * Returns 0 on success, -1 on error (both commands are cleaned up).
 */
int parse(char *line, Command *cmd1, Command *cmd2, int *has_pipe) {
  char *tokens[MAX_ARGS + 8];
  int ntok = 0;
  int pipe_pos = -1;
  char *copy;

  init_command(cmd1);
  init_command(cmd2);
  *has_pipe = 0;

  copy = strdup(line);
  if (!copy) {
    perror("strdup");
    return -1;
  }

  /* split on whitespace into a temporary token array */
  char *tok = strtok(copy, " \t");
  while (tok && ntok < MAX_ARGS + 7) {
    tokens[ntok++] = tok;
    tok = strtok(NULL, " \t");
  }
  tokens[ntok] = NULL;

  if (ntok == 0) {
    free(copy);
    return 0;
  }

  /* locate the first pipe token */
  for (int i = 0; i < ntok; i++) {
    if (strcmp(tokens[i], "|") == 0) {
      pipe_pos = i;
      *has_pipe = 1;
      break;
    }
  }

  int end1 = (pipe_pos >= 0) ? pipe_pos : ntok;

  if (fill_command(tokens, 0, end1, cmd1) < 0)
    goto err;

  if (pipe_pos >= 0 && fill_command(tokens, pipe_pos + 1, ntok, cmd2) < 0)
    goto err;

  free(copy);
  return 0;

err:
  free(copy);
  free_command(cmd1);
  free_command(cmd2);
  return -1;
}
