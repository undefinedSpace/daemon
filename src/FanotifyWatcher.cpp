#include "FanotifyWatcher.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/fanotify.h>
#include <poll.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/fanotify.h>
#include <sys/statfs.h>
#include <unistd.h>

#include "RootMonitor.h"

extern RootMonitor *rmProject;

namespace {
const unsigned long long events = FAN_CREATE | FAN_DELETE | FAN_MOVED_FROM |
                                  FAN_MOVED_TO | FAN_MODIFY | FAN_CLOSE_WRITE |
                                  FAN_ATTRIB | FAN_EVENT_ON_CHILD | FAN_ONDIR;

std::string HandleKey(const void *fsid, const file_handle *handle)
{
  std::string key(reinterpret_cast<const char *>(fsid), sizeof(__kernel_fsid_t));
  key.append(reinterpret_cast<const char *>(&handle->handle_type), sizeof(handle->handle_type));
  key.append(reinterpret_cast<const char *>(handle->f_handle), handle->handle_bytes);
  return key;
}
}

FanotifyWatcher::FanotifyWatcher() : fanotifyFd(-1)
{
  pthread_mutex_init(&mutex, NULL);
  // Named directory-entry events include the parent's file handle. This
  // requires a filesystem with file-handle support and suitable privileges.
  fanotifyFd = fanotify_init(FAN_CLASS_NOTIF | FAN_NONBLOCK | FAN_CLOEXEC |
                             FAN_REPORT_DFID_NAME, O_RDONLY | O_CLOEXEC);
  if(fanotifyFd < 0)
    perror("fanotify_init");
}

FanotifyWatcher::~FanotifyWatcher()
{
  if(fanotifyFd >= 0)
    close(fanotifyFd);
  pthread_mutex_destroy(&mutex);
}

bool FanotifyWatcher::IsReady() const { return fanotifyFd >= 0; }

bool FanotifyWatcher::AddDirectory(SomeDirectory *directory)
{
  if(!IsReady() || directory == NULL || directory->GetDirFd() < 0)
    return false;

  char *path = directory->GetFullPath();
  if(path == NULL)
    return false;

  // name_to_handle_at() tells us the same opaque handle that fanotify puts
  // into FAN_EVENT_INFO_TYPE_DFID_NAME records.
  union {
    file_handle handle;
    char storage[sizeof(file_handle) + 128];
  } buffer;
  memset(&buffer, 0, sizeof(buffer));
  buffer.handle.handle_bytes = sizeof(buffer.storage) - sizeof(file_handle);
  int mountId;
  struct statfs fs;
  bool ok = name_to_handle_at(AT_FDCWD, path, &buffer.handle, &mountId, 0) == 0 &&
            fstatfs(directory->GetDirFd(), &fs) == 0;
  if(!ok)
  {
    perror("fanotify directory handle");
    delete [] path;
    return false;
  }
  static_assert(sizeof(fs.f_fsid) == sizeof(__kernel_fsid_t), "fanotify fsid size mismatch");
  std::string key = HandleKey(&fs.f_fsid, &buffer.handle);

  pthread_mutex_lock(&mutex);
  if(fanotify_mark(fanotifyFd, FAN_MARK_ADD | FAN_MARK_ONLYDIR, events,
                   AT_FDCWD, path) < 0)
  {
    perror("fanotify_mark");
    ok = false;
  }
  else
  {
    Directory entry = {key, directory->GetDirFd()};
    directories.push_back(entry);
  }
  pthread_mutex_unlock(&mutex);
  delete [] path;
  return ok;
}

void FanotifyWatcher::RemoveDirectory(int fd)
{
  pthread_mutex_lock(&mutex);
  for(std::vector<Directory>::iterator it = directories.begin(); it != directories.end(); ++it)
    if(it->fd == fd)
    {
      if(fanotify_mark(fanotifyFd, FAN_MARK_REMOVE, events, fd, NULL) < 0 &&
         errno != ENOENT)
        perror("fanotify remove mark");
      directories.erase(it);
      break;
    }
  pthread_mutex_unlock(&mutex);
}

void FanotifyWatcher::Run()
{
  alignas(struct fanotify_event_metadata) char buffer[64 * 1024];
  struct pollfd pollFd = {fanotifyFd, POLLIN, 0};
  for(;;)
  {
    if(poll(&pollFd, 1, -1) <= 0)
    {
      if(errno != EINTR) perror("fanotify poll");
      continue;
    }
    ssize_t length = read(fanotifyFd, buffer, sizeof(buffer));
    if(length < 0)
    {
      if(errno != EAGAIN && errno != EINTR) perror("fanotify read");
      continue;
    }

    // Coalesce notifications from one read. Unknown event formats, file
    // modifications and queue overflow fall back to scanning every directory.
    bool scanAll = false;
    std::vector<int> changed;
    for(char *cursor = buffer; cursor + sizeof(fanotify_event_metadata) <= buffer + length; )
    {
      fanotify_event_metadata *event = reinterpret_cast<fanotify_event_metadata *>(cursor);
      if(event->event_len < FAN_EVENT_METADATA_LEN || cursor + event->event_len > buffer + length)
      {
        scanAll = true;
        break;
      }
      if(event->fd != FAN_NOFD)
        close(event->fd);
      if(event->vers != FANOTIFY_METADATA_VERSION || (event->mask & FAN_Q_OVERFLOW))
        scanAll = true;
      else
      {
        bool found = false;
        for(char *info = cursor + event->metadata_len;
            info + sizeof(fanotify_event_info_header) <= cursor + event->event_len; )
        {
          fanotify_event_info_header *header = reinterpret_cast<fanotify_event_info_header *>(info);
          if(header->len < sizeof(*header) || info + header->len > cursor + event->event_len)
            break;
          if(header->info_type == FAN_EVENT_INFO_TYPE_DFID_NAME &&
             header->len >= offsetof(fanotify_event_info_fid, handle) + sizeof(file_handle))
          {
            fanotify_event_info_fid *fid = reinterpret_cast<fanotify_event_info_fid *>(info);
            file_handle *handle = reinterpret_cast<file_handle *>(fid->handle);
            if(offsetof(fanotify_event_info_fid, handle) + sizeof(file_handle) +
               handle->handle_bytes <= header->len)
            {
              std::string key = HandleKey(&fid->fsid, handle);
              pthread_mutex_lock(&mutex);
              for(const Directory &entry : directories)
                if(entry.key == key)
                {
                  changed.push_back(entry.fd);
                  found = true;
                  break;
                }
              pthread_mutex_unlock(&mutex);
            }
          }
          info += header->len;
        }
        if(!found) scanAll = true;
      }
      cursor += event->event_len;
    }

    pthread_mutex_lock(&RootMonitor::mScanMutex);
    if(RootMonitor::pdlList != NULL && rmProject != NULL)
    {
      if(scanAll)
        RootMonitor::pdlList->CompareAll(RootMonitor::pszToken);
      else
        for(int fd : changed)
        {
          SomeDirectory *directory = RootMonitor::pdlList->GetDirectory(fd);
          if(directory != NULL && !directory->IsSnapshotNeeded())
            directory->CompareSnapshots(RootMonitor::pszToken);
        }
    }
    pthread_mutex_unlock(&RootMonitor::mScanMutex);
  }
}
