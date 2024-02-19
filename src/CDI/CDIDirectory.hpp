#ifndef CDI_CDIDIRECTORY_HPP
#define CDI_CDIDIRECTORY_HPP

class CDIDisc;
#include "CDIFile.hpp"

#include <functional>
#include <map>
#include <string>
#include <string_view>

/** \brief CD-I directory on disc.
 *
 * The root directory has a nameSize of 0 and a name of "".
 */
class CDIDirectory
{
public:
    const uint8_t nameSize;
    const std::string name;
    uint32_t LBN;
    const uint16_t parentDirectory;
    const uint16_t relOffset;

    CDIDirectory() = delete;
    CDIDirectory(uint8_t namesize, std::string dirName, uint32_t lbn, uint16_t parent, uint16_t offset);

    void Clear();
    void LoadContent(CDIDisc& disc);
    const CDIFile* GetFile(std::string filename);

    std::stringstream GetChildrenTree() const;
    void ExportAudio(std::string basePath) const;
    void ExportFiles(std::string basePath) const;
    void ExportRawVideo(std::string basePath) const;

    void ForEachFile(const std::string& path, std::function<void(std::string_view, const CDIFile&)> f) const;

private:
    std::map<std::string, CDIFile> files;
    std::map<std::string, CDIDirectory> subdirectories;
};

#endif // CDI_CDIDIRECTORY_HPP
