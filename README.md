# Mini Shell

A small Unix-style shell written in C using POSIX system calls. Built for
systems programming interview preparation.

---

## Architecture

```
[ User input ]
      |
      v
  [ parser.c ]  ------>  [ builtins.c ]
      |                   cd / pwd / help / exit
      v
  [ executor.c ]
   fork / execvp / pipe / dup2
      |
      v
  [ Output ]
```

---

## Build

```bash
make
```

Requires GCC on a POSIX system (Linux, macOS, WSL).

---

## Run

```bash
./mini-shell
```

---

## Supported Features

| Feature              | Example                    |
|----------------------|----------------------------|
| External commands    | `ls -la`                   |
| Built-in: cd         | `cd /tmp`                  |
| Built-in: pwd        | `pwd`                      |
| Built-in: help       | `help`                     |
| Built-in: exit       | `exit`                     |
| Output redirection   | `ls > out.txt`             |
| Input redirection    | `sort < data.txt`          |
| Single pipe          | `ls | grep .c`             |
| EOF (Ctrl+D)         | exits cleanly              |

---

## Limitations

- Only one pipe is supported
- No background execution (`&`)
- No signal handling (Ctrl+C kills the shell)
- No command history or tab completion
- No wildcard expansion or variable substitution

---

## Key System Calls Explained

### `fork()`
Duplicates the current process. The parent (shell) gets the child's PID; the
child gets 0. The child then calls `exec` to run a new program while the
parent waits.

### `execvp()`
Replaces the child's process image with a new program found on `$PATH`.
It never returns unless an error occurs.

### `pipe()`
Creates a unidirectional byte channel. `pipefd[1]` is the write end and
`pipefd[0]` is the read end. Data written to the write end is buffered by the
kernel until it is read from the read end.

### `dup2(fd, target)`
Copies file descriptor `fd` onto `target` (closing `target` first). Used to
rewire stdin/stdout to files or pipe ends before calling `execvp`, so the new
program reads/writes the right place without knowing it was redirected.

---

## Future Improvements

- Multiple pipes (`cmd1 | cmd2 | cmd3`)
- Background execution (`cmd &`)
- Signal handling (`SIGINT`, `SIGCHLD`)
- Command history (up-arrow)
- Tab completion
