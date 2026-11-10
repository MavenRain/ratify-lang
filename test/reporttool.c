/* Checks that langc report and fmt observe a buffered write error (a port of
 * permit-lang test/reporttool.c, S11 U3). */
#include <errno.h>
#include <fcntl.h>
#include <spawn.h>
#include <stdio.h>
#include <sys/wait.h>

extern char **environ;

/* The same valid program must pass with writable stdout and fail with
 * read-only stdout for COMMAND. stdout is a buffered stream in langc. */
static int run(char *compiler, char *command, char *program, int mode, int want) {
  char *args[] = {compiler, command, program, NULL};
  posix_spawn_file_actions_t actions;
  pid_t pid, waited;
  int status, ok;
  if (posix_spawn_file_actions_init(&actions) != 0)
    return 0;
  ok = posix_spawn_file_actions_addopen(&actions, 1, "/dev/null", mode, 0) == 0 &&
       posix_spawn_file_actions_addopen(&actions, 2, "/dev/null", O_WRONLY, 0) == 0 &&
       posix_spawn(&pid, compiler, &actions, NULL, args, environ) == 0;
  posix_spawn_file_actions_destroy(&actions);
  if (!ok)
    return 0;
  do {
    waited = waitpid(pid, &status, 0);
  } while (waited < 0 && errno == EINTR);
  return waited == pid && WIFEXITED(status) && WEXITSTATUS(status) == want;
}

/* Runs the 2 checks of COMMAND and writes its count line. Returns 1, or 0
 * after a FAIL line. */
static int check(char *compiler, char *command, char *program) {
  if (!run(compiler, command, program, O_WRONLY, 0) || !run(compiler, command, program, O_RDONLY, 1)) {
    fprintf(stderr, "FAIL %s write error: wrong exit status\n", command);
    return 0;
  }
  printf("%s write errors: 2 checked\n", command);
  return 1;
}

int main(int argc, char **argv) {
  if (argc != 3) {
    fputs("usage: reporttool LANGC PROG\n", stderr);
    return 2;
  }
  return check(argv[1], "report", argv[2]) && check(argv[1], "fmt", argv[2]) ? 0 : 1;
}
