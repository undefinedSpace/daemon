#pragma once

// Raise the per-process soft file-descriptor limit to the maximum permitted
// by its hard limit (or by the kernel when the hard limit is unlimited).
bool ConfigureOpenFileLimit();
