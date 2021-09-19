#include "filesystem.hpp"

#include <filesystem>

/** \brief Create a directory (intermediate directories are created if they don't exists).
 *
 * \param path The path to the new directory to create.
 * \return true if the directory has been created or already exists, false otherwise.
 */
bool createDirectories(const std::string& path)
{
    if(std::filesystem::exists(path))
        return true;
    else
        return std::filesystem::create_directories(path);
}
