#include "FileUuid.h"

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/xattr.h>
#include <unistd.h>

#include <fstream>
#include <string>
#include <vector>

static std::string Uuid(const std::string &path)
{
  char uuid[37];
  assert(EnsureFileUuid(path.c_str(), uuid));
  return uuid;
}

static void CopyAttribute(const std::string &from, const std::string &to,
                          const char *name)
{
  const ssize_t size = getxattr(from.c_str(), name, NULL, 0);
  assert(size > 0);
  std::vector<char> value(size);
  assert(getxattr(from.c_str(), name, value.data(), value.size()) == size);
  assert(setxattr(to.c_str(), name, value.data(), value.size(), 0) == 0);
}

int main()
{
  const char *tmpdir = getenv("TMPDIR");
  const std::string path = std::string(tmpdir ? tmpdir : "/tmp") + "/undefined-uuid-XXXXXX";
  std::vector<char> dirname(path.begin(), path.end());
  dirname.push_back('\0');
  assert(mkdtemp(dirname.data()) != NULL);
  const std::string root(dirname.data());
  const std::string original = root + "/original";
  const std::string copy = root + "/copy";
  const std::string link = root + "/link";
  const std::string renamed = root + "/renamed";

  std::ofstream(original).put('x');
  const std::string sourceUuid = Uuid(original);
  std::ofstream(copy).put('x');
  CopyAttribute(original, copy, "user.undefinedSpace.id");
  CopyAttribute(original, copy, "user.undefinedSpace.identity");

  // A copy made while the monitor is stopped must receive a new UUID on scan.
  const std::string copyUuid = Uuid(copy);
  assert(copyUuid != sourceUuid);
  assert(Uuid(copy) == copyUuid);
  assert(Uuid(original) == sourceUuid);

  assert(linkat(AT_FDCWD, original.c_str(), AT_FDCWD, link.c_str(), 0) == 0);
  assert(Uuid(link) == sourceUuid);
  assert(rename(original.c_str(), renamed.c_str()) == 0);
  assert(Uuid(renamed) == sourceUuid);

  assert(unlink(link.c_str()) == 0);
  assert(unlink(renamed.c_str()) == 0);
  assert(unlink(copy.c_str()) == 0);
  assert(rmdir(root.c_str()) == 0);
}
