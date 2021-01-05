#include "filesystem.hpp"

#ifdef USE_STD_FILESYSTEM
    #ifdef FILESYSTEM_EXPERIMENTAL
        #include <experimental/filesystem>
    #else
        #include <filesystem>
    #endif // FILESYSTEM_EXPERIMENTAL
#else
    #include <wx/filefn.h>
#endif // USE_STD_FILESYSTEM

/** \brief Create a directory (intermediate directories are created if they don't exists).
 *
 * \param path The path to the new directory to create.
 * \return true if the directory has been created or already exists, false otherwise.
 */
bool createDirectories(const std::string& path)
{
#ifdef USE_STD_FILESYSTEM
#ifdef FILESYSTEM_EXPERIMENTAL
    return std::experimental::filesystem::create_directories(path);
#else
    return std::filesystem::create_directories(path);
#endif // FILESYSTEM_EXPERIMENTAL
#else
    std::string p = path;
    std::string newPath;
    do
    {
        const size_t pos = p.find('/');
        newPath += pos == std::string::npos ? p : p.substr(0, pos+1);

        if(!wxDirExists(newPath))
            if(!wxMkdir(newPath))
                return false;

        p = p.size() > 1 ? p.substr(pos+1) : "";

    } while(p.length() > 0);

    return true;
#endif // USE_STD_FILESYSTEM
}
