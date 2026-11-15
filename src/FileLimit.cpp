#include "FileLimit.h"

#include <stdio.h>
#include <sys/resource.h>

bool ConfigureOpenFileLimit()
{
  struct rlimit limit;
  if(getrlimit(RLIMIT_NOFILE, &limit) < 0)
  {
    perror("getrlimit RLIMIT_NOFILE");
    return false;
  }

  rlim_t target = limit.rlim_max;
  if(target == RLIM_INFINITY)
  {
    FILE *file = fopen("/proc/sys/fs/nr_open", "r");
    if(file == NULL)
    {
      perror("open /proc/sys/fs/nr_open");
      return false;
    }
    unsigned long long kernelLimit;
    const int read = fscanf(file, "%llu", &kernelLimit);
    fclose(file);
    if(read != 1)
    {
      fprintf(stderr, "Cannot read kernel file-descriptor limit from /proc/sys/fs/nr_open\n");
      return false;
    }
    target = static_cast<rlim_t>(kernelLimit);
  }

  if(limit.rlim_cur != target)
  {
    limit.rlim_cur = target;
    if(setrlimit(RLIMIT_NOFILE, &limit) < 0)
    {
      perror("setrlimit RLIMIT_NOFILE");
      return false;
    }
  }
  fprintf(stderr, "Open-file limit: soft=%llu, hard=%llu\n",
          static_cast<unsigned long long>(limit.rlim_cur),
          static_cast<unsigned long long>(limit.rlim_max));
  return true;
}
