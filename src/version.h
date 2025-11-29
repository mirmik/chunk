#pragma once

namespace chunk
{
    inline constexpr int version_major = 0;
    inline constexpr int version_minor = 3;
    inline constexpr int version_patch = 0;

    inline constexpr const char *version_string = "0.3.0";
} // namespace chunk

/* CHANGELIST

0.3.0 
    The program no longer writes the result of each command to disk, 
    but first compiles a new file in its memory and 
    writes changes to all files at once if the patch was applied successfully.
0.2.0 Added append and prepend commands, object refactoring has been performed
0.1.0 Initial

*/