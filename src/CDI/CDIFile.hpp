#ifndef CDIFILE_HPP
#define CDIFILE_HPP

class CDIFile;

#include <string>

#include "CDIDisk.hpp"

enum AudioCodingInformation
{
    emphasis = 0b01000000,
    bps      = 0b00110000, // bits per sample
    sf       = 0b00001100, // sampling frequency
    ms       = 0b00000011  // mono/stereo
};

class CDIFile
{
    CDIDisk& disk;

public:
    const uint32_t fileLBN;
    const uint32_t filesize;
    const uint8_t nameSize;
    const std::string filename;
    const uint16_t attributes;
    const uint8_t fileNumber;
    const uint16_t parent;

    CDIFile() = delete;
    CDIFile(CDIDisk& cdidisk, uint32_t lbn, uint32_t filesize, uint8_t namesize, std::string name, uint16_t attr, uint8_t fileNumber, uint16_t parentRelpos);

    void ExportAudio(std::string directoryPath);
    void ExportFile(std::string directoryPath);
    char* GetFileContent(uint32_t& size);
};

struct WAVHeader
{
    uint16_t channelNumber;
    uint32_t frequency;
    uint16_t bitsPerSample;
};

#endif // CDIFILE_HPP
