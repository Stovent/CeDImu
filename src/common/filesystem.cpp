#include "filesystem.hpp"

#ifdef USE_STD_FILESYSTEM
#include <filesystem>
#else
#include <wx/filefn.h>
#endif // USE_STD_FILESYSTEM

bool createDirectory(const std::string& path)
{
#ifdef USE_STD_FILESYSTEM
    if(!std::filesystem::create_directory(path))
        return false;
#else
    if(!wxDirExists(path))
        if(!wxMkdir(path))
            return false;
#endif // USE_STD_FILESYSTEM

    return true;
}
