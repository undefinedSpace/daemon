#pragma once

#include <pthread.h>
#include <string>
#include <vector>

class SomeDirectory;

// Reports the directory containing a changed entry. Events without a parent
// identifier (for example file modifications) require a full reconciliation.
class FanotifyWatcher
{
  struct Directory {
    std::string key;
    int fd;
  };

  int fanotifyFd;
  pthread_mutex_t mutex;
  std::vector<Directory> directories;

public:
  FanotifyWatcher();
  ~FanotifyWatcher();
  bool IsReady() const;
  bool AddDirectory(SomeDirectory *directory);
  void RemoveDirectory(int fd);
  void Run();
};
