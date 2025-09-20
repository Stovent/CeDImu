#ifndef EXPORT_HPP
#define EXPORT_HPP

#include "CDI/Video/VideoCommon.hpp"
#include "CDI/CDIDisc.hpp"

#include <span>

void splitARGB(std::span<const Video::Pixel> pixels, uint8_t* alpha, uint8_t* rgb);
void exportVideo(CDIDisc& disc, std::string exportDir);

#endif // EXPORT_HPP
