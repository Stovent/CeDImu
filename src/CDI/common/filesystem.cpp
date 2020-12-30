#include "filesystem.hpp"

#ifdef USE_STD_FILESYSTEM
#include <filesystem>
#else
#include <wx/filefn.h>
#endif // USE_STD_FILESYSTEM

/** \brief Create a directory (intermediate directories are created if they don't exists).
 *
 * \param path The path to the new directory to create.
 * \return true if the directory has been created or already exists, false otherwise.
 */
bool createDirectories(std::string path)
{
#ifdef USE_STD_FILESYSTEM
    if(!std::filesystem::create_directories(path))
        return false;
#else
    std::string newPath;
    do
    {
        const size_t pos = path.find('/');
        newPath += pos == std::string::npos ? path : path.substr(0, pos+1);

        if(!wxDirExists(newPath))
            if(!wxMkdir(newPath))
                return false;

        path = path.size() > 1 ? path.substr(pos+1) : "";

    } while(path.length() > 0);
#endif // USE_STD_FILESYSTEM

    return true;
}
