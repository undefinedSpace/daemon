#include "FileLimit.h"

#include <assert.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

int main()
{
  struct rlimit original;
  assert(getrlimit(RLIMIT_NOFILE, &original) == 0);
  const pid_t child = fork();
  assert(child >= 0);
  if(child == 0)
  {
    // Simulate a shell with a low soft limit and a higher hard limit.
    struct rlimit reduced = {64, 128};
    if(setrlimit(RLIMIT_NOFILE, &reduced) != 0) _exit(2);
    if(!ConfigureOpenFileLimit()) _exit(3);
    struct rlimit actual;
    if(getrlimit(RLIMIT_NOFILE, &actual) != 0 ||
       actual.rlim_cur != 128 || actual.rlim_max != 128) _exit(4);
    _exit(0);
  }
  int status;
  assert(waitpid(child, &status, 0) == child);
  assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
  struct rlimit after;
  assert(getrlimit(RLIMIT_NOFILE, &after) == 0);
  assert(after.rlim_cur == original.rlim_cur && after.rlim_max == original.rlim_max);
}
