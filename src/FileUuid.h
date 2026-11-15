#pragma once

// A UUID belongs to the filesystem object, not its pathname. The xattr is
// preserved by rename within a filesystem; copying/replacing creates a new
// object and requires a new UUID unless the application explicitly transfers it.
bool EnsureFileUuid(const char *path, char out[37]);
