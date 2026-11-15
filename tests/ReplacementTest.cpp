#include "DirSnapshot.h"
#include "RootMonitor.h"
#include "JSONService.h"
#include "SomeDirectory.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <fstream>
#include <string>
#include <vector>

RootMonitor *rmProject = NULL;

int main()
{
  const char *tmp = getenv("TMPDIR");
  std::string pattern = std::string(tmp ? tmp : "/tmp") + "/undefined-replace-XXXXXX";
  std::vector<char> buffer(pattern.begin(), pattern.end());
  buffer.push_back('\0');
  assert(mkdtemp(buffer.data()) != NULL);
  std::string root(buffer.data()), before = root + "/before", after = root + "/after";
  assert(mkdir(before.c_str(), 0700) == 0);
  assert(mkdir(after.c_str(), 0700) == 0);
  const std::string scan = root + "/scan";
  const std::string link = scan + "/link";
  assert(mkdir(scan.c_str(), 0700) == 0);
  assert(symlink("../before/same.txt", link.c_str()) == 0);
  std::ofstream(before + "/same.txt") << "original";
  std::ofstream(after + "/same.txt") << "replacement";
  {
    char emptyPath[] = "";
    auto rootData = new DirSnapshot::FileData(scan.c_str(), emptyPath, NULL, false);
    SomeDirectory directory(rootData, NULL, false);
    FILE *diagnostics = tmpfile();
    assert(diagnostics != NULL);
    const int savedStderr = dup(STDERR_FILENO);
    assert(savedStderr >= 0);
    fflush(stderr);
    assert(dup2(fileno(diagnostics), STDERR_FILENO) >= 0);
    directory.MakeSnapshot();
    fflush(stderr);
    assert(dup2(savedStderr, STDERR_FILENO) >= 0);
    close(savedStderr);
    rewind(diagnostics);
    char message[2048] = {};
    assert(fgets(message, sizeof(message), diagnostics) != NULL);
    assert(std::string(message).find(link) != std::string::npos);
    assert(std::string(message).find("failed") != std::string::npos);
    assert(std::string(message).find("error! rmProject=") == std::string::npos);
    fclose(diagnostics);
  }
  {
    DirSnapshot oldFiles, newFiles;
    auto oldFile = oldFiles.AddFile("same.txt", &before[0], true);
    auto newFile = newFiles.AddFile("same.txt", &after[0], true);
    assert(oldFile && newFile);
    assert(std::string(oldFile->szUuid) != newFile->szUuid);
    oldFiles.CompareSnapshots(&newFiles, false);
    DirSnapshot::SnapshotComparison result;
    oldFiles.GetResult(&result);
    assert(result.rocResult == IS_REPLACED);
    assert(result.pfdData == newFile);
    assert(result.pfdReplaced == oldFile);
    DirSnapshot::FileData parent;
    std::string oldUuid = oldFile->szUuid;
    FSChange json(CURRENT_SERVICE, newFile, &parent, IS_REPLACED, NULL, oldUuid.c_str());
    assert(std::string(json.pszChanges).find("\"event\":\"IS_REPLACED\"") != std::string::npos);
    assert(std::string(json.pszChanges).find("\"old_uuid\":\"" + oldUuid + "\"") != std::string::npos);
    assert(std::string(json.pszChanges).find("\"uuid\":\"" + std::string(newFile->szUuid) + "\"") != std::string::npos);
    oldFiles.GetResult(&result);
    assert(result.rocResult == IS_EMPTY);

    // An in-place edit retains the object UUID and must not be a replacement.
    DirSnapshot sameFile;
    sameFile.AddFile("same.txt", &before[0], true);
    oldFiles.CompareSnapshots(&sameFile, false);
    oldFiles.GetResult(&result);
    assert(result.rocResult == IS_EQUAL);

    DirSnapshot empty;
    oldFiles.CompareSnapshots(&empty, false);
    oldFiles.GetResult(&result);
    assert(result.rocResult == IS_DELETED);
    oldFiles.GetResult(&result);
    assert(result.rocResult == IS_EMPTY);
    empty.CompareSnapshots(&newFiles, false);
    empty.GetResult(&result);
    assert(result.rocResult == IS_CREATED);
  }
  assert(unlink((before + "/same.txt").c_str()) == 0);
  assert(unlink((after + "/same.txt").c_str()) == 0);
  assert(unlink(link.c_str()) == 0);
  assert(rmdir(scan.c_str()) == 0);
  assert(rmdir(before.c_str()) == 0);
  assert(rmdir(after.c_str()) == 0);
  assert(rmdir(root.c_str()) == 0);
}
