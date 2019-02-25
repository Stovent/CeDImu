#include "CDIFile.hpp"

CDIFile::CDIFile(uint32_t lbn, uint32_t filesize, uint8_t namesize, std::string filename, uint16_t attr, uint8_t filenumber, uint16_t parentRelpos)
{
    nameSize = namesize;
    name = filename;
    size = filesize;
    LBN = lbn;
    attributes = attr;
    parent = parentRelpos;
    fileNumber = filenumber;
}
