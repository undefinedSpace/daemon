#include "FileUuid.h"

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/random.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/xattr.h>

namespace {
const char uuidAttribute[] = "user.undefinedSpace.id";
const char identityAttribute[] = "user.undefinedSpace.identity";

// The filesystem ID and birth time distinguish copies, including ones made
// with cp -a. Unlike st_dev, statfs.f_fsid is stable across remounts on ext4.
struct Identity {
  uint64_t fsid;
  uint64_t inode;
  int64_t birthSeconds;
  uint32_t birthNanoseconds;
  uint32_t version;
};
static_assert(sizeof(Identity) == 32, "unexpected xattr identity layout");

bool CurrentIdentity(const char *path, Identity &identity)
{
  struct stat file;
  struct statfs fs;
  if(lstat(path, &file) < 0 || statfs(path, &fs) < 0)
  {
    perror("file identity stat");
    return false;
  }
  static_assert(sizeof(fs.f_fsid) == sizeof(identity.fsid), "unexpected filesystem ID size");
  memset(&identity, 0, sizeof(identity));
  memcpy(&identity.fsid, &fs.f_fsid, sizeof(identity.fsid));
  identity.inode = file.st_ino;
  identity.version = 1;

  struct statx extended;
  if(statx(AT_FDCWD, path, AT_SYMLINK_NOFOLLOW, STATX_BTIME, &extended) == 0 &&
     (extended.stx_mask & STATX_BTIME))
  {
    identity.birthSeconds = extended.stx_btime.tv_sec;
    identity.birthNanoseconds = extended.stx_btime.tv_nsec;
  }
  return true;
}

bool Valid(const char *value)
{
  for(int i = 0; i < 36; ++i)
  {
    const char c = value[i];
    if(i == 8 || i == 13 || i == 18 || i == 23)
    {
      if(c != '-') return false;
    }
    else if(!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')))
      return false;
  }
  return value[36] == '\0';
}

bool NewUuid(char value[37])
{
  unsigned char bytes[16];
  if(getrandom(bytes, sizeof(bytes), 0) != sizeof(bytes))
  {
    perror("getrandom UUID");
    return false;
  }
  bytes[6] = (bytes[6] & 0x0f) | 0x40;
  bytes[8] = (bytes[8] & 0x3f) | 0x80;
  snprintf(value, 37,
           "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
           bytes[0],bytes[1],bytes[2],bytes[3],bytes[4],bytes[5],bytes[6],bytes[7],
           bytes[8],bytes[9],bytes[10],bytes[11],bytes[12],bytes[13],bytes[14],bytes[15]);
  return true;
}
}

bool EnsureFileUuid(const char *path, char out[37])
{
  Identity current;
  if(!CurrentIdentity(path, current)) return false;

  char value[37];
  ssize_t size = lgetxattr(path, uuidAttribute, value, sizeof(value));
  const bool hadUuid = size == 36;
  if(hadUuid)
  {
    value[36] = '\0';
    if(!Valid(value))
    {
      fprintf(stderr, "Invalid UUID xattr: %s\n", path);
      return false;
    }
  }
  else if(size >= 0 || errno != ENODATA)
  {
    perror("read UUID xattr");
    return false;
  }

  Identity stored;
  const ssize_t identitySize = lgetxattr(path, identityAttribute, &stored, sizeof(stored));
  const bool hadIdentity = identitySize == sizeof(stored) && stored.version == 1;
  if(identitySize >= 0 && !hadIdentity)
  {
    fprintf(stderr, "Invalid identity xattr: %s\n", path);
    return false;
  }
  if(identitySize < 0 && errno != ENODATA)
  {
    perror("read identity xattr");
    return false;
  }
  if(hadUuid && hadIdentity && memcmp(&stored, &current, sizeof(stored)) == 0)
  {
    memcpy(out, value, sizeof(value));
    return true;
  }

  // No identity marker means an xattr written by the earlier daemon version.
  // Bind that UUID to the current inode on its first observation. A copied
  // marker, however, proves this is a distinct object and needs a new UUID.
  if(!hadUuid || hadIdentity)
  {
    if(!NewUuid(value)) return false;
    if(lsetxattr(path, uuidAttribute, value, 36,
                 hadUuid ? XATTR_REPLACE : XATTR_CREATE) < 0)
    {
      if(errno == EEXIST) return EnsureFileUuid(path, out);
      perror("write UUID xattr");
      return false;
    }
  }

  if(lsetxattr(path, identityAttribute, &current, sizeof(current),
               hadIdentity ? XATTR_REPLACE : XATTR_CREATE) < 0)
  {
    if(errno == EEXIST) return EnsureFileUuid(path, out);
    perror("write identity xattr");
    return false;
  }
  memcpy(out, value, sizeof(value));
  return true;
}
