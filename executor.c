#define _POSIX_C_SOURCE 200809L

#include "shell.h"

/*
 * Redirect stdin/stdout as specified by cmd.
 * Must be called only in the child process (after fork).
 * Returns 0 on success, -1 on error.
 */
static int apply_redirections(Command *cmd) {
  if (cmd->input_file) {
    /* open the file for reading */
    int fd = open(cmd->input_file, O_RDONLY);
    if (fd < 0) {
      perror(cmd->input_file);
      return -1;
    }

    /* make stdin point to fd; the original fd is then redundant */
    if (dup2(fd, STDIN_FILENO) < 0) {
      perror("dup2");
      close(fd);
      return -1;
    }
    close(fd);
  }

  if (cmd->output_file) {
    /* create or truncate the file, then open for writing */
    int fd = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
      perror(cmd->output_file);
      return -1;
    }

    /* make stdout point to fd */
    if (dup2(fd, STDOUT_FILENO) < 0) {
      perror("dup2");
      close(fd);
      return -1;
    }
    close(fd);
  }

  return 0;
}

/* Execute a single external command with optional I/O redirection. */
static void run_single(Command *cmd) {
  pid_t pid = fork(); /* duplicate the shell process */
  if (pid < 0) {
    perror("fork");
    return;
  }

  if (pid == 0) {
    /* child process */
    if (apply_redirections(cmd) < 0)
      exit(1);

    /* replace child image with the requested program */
    execvp(cmd->argv[0], cmd->argv);

    /* execvp only returns on failure */
    perror(cmd->argv[0]);
    exit(1);
  }

  /* parent waits for the child to finish */
  if (waitpid(pid, NULL, 0) < 0)
    perror("waitpid");
}

/* Execute cmd1 | cmd2 — connect them through an anonymous pipe. */
static void run_pipe(Command *cmd1, Command *cmd2) {
  int pipefd[2]; /* [0] = read end, [1] = write end */

  /* create the pipe; the kernel allocates a buffer between the two FDs */
  if (pipe(pipefd) < 0) {
    perror("pipe");
    return;
  }

  /* --- left-hand side: cmd1 writes into the pipe --- */
  pid_t pid1 = fork();
  if (pid1 < 0) {
    perror("fork");
    close(pipefd[0]);
    close(pipefd[1]);
    return;
  }

  if (pid1 == 0) {
    close(pipefd[0]); /* child 1 does not read from the pipe */

    /* wire stdout to the write end so cmd1's output enters the pipe */
    if (dup2(pipefd[1], STDOUT_FILENO) < 0) {
      perror("dup2");
      exit(1);
    }
    close(pipefd[1]); /* dup2 copied it; original FD no longer needed */

    if (apply_redirections(cmd1) < 0)
      exit(1);
    execvp(cmd1->argv[0], cmd1->argv);
    perror(cmd1->argv[0]);
    exit(1);
  }

  /* --- right-hand side: cmd2 reads from the pipe --- */
  pid_t pid2 = fork();
  if (pid2 < 0) {
    perror("fork");
    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, NULL, 0);
    return;
  }

  if (pid2 == 0) {
    close(pipefd[1]); /* child 2 does not write into the pipe */

    /* wire stdin to the read end so cmd2 receives cmd1's output */
    if (dup2(pipefd[0], STDIN_FILENO) < 0) {
      perror("dup2");
      exit(1);
    }
    close(pipefd[0]);

    if (apply_redirections(cmd2) < 0)
      exit(1);
    execvp(cmd2->argv[0], cmd2->argv);
    perror(cmd2->argv[0]);
    exit(1);
  }

  /* parent: close both ends (children own them now) and wait */
  close(pipefd[0]);
  close(pipefd[1]);
  if (waitpid(pid1, NULL, 0) < 0)
    perror("waitpid");
  if (waitpid(pid2, NULL, 0) < 0)
    perror("waitpid");
}

void execute(Command *cmd1, Command *cmd2, int has_pipe) {
  if (has_pipe)
    run_pipe(cmd1, cmd2);
  else
    run_single(cmd1);
}
