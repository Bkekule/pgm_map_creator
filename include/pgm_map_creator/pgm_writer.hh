#ifndef PGM_MAP_CREATOR_PGM_WRITER_HH
#define PGM_MAP_CREATOR_PGM_WRITER_HH

#include <cstdint>
#include <string>
#include <vector>

namespace pgm_map_creator {

/// \brief Write a grayscale image buffer to a PGM (P2) file.
/// \param[in] _filename Output path (without .pgm extension — it will be
/// appended)
/// \param[in] _image 2D image buffer [rows][cols] of grayscale values
/// \param[in] _width Number of columns
/// \param[in] _height Number of rows
void WritePgm(const std::string &_filename,
              const std::vector<std::vector<uint8_t>> &_image, int _width,
              int _height);

} // namespace pgm_map_creator

#endif // PGM_MAP_CREATOR_PGM_WRITER_HH
