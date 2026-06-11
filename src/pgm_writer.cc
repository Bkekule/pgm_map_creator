#include "pgm_map_creator/pgm_writer.hh"

#include <fstream>

namespace pgm_map_creator {

void WritePgm(const std::string &_filename,
              const std::vector<std::vector<uint8_t>> &_image, int _width,
              int _height) {
  std::ofstream ofs(_filename + ".pgm");
  ofs << "P2" << '\n';
  ofs << _width << ' ' << _height << '\n';
  ofs << 255 << '\n';

  for (int row = 0; row < _height; ++row) {
    for (int col = 0; col < _width; ++col) {
      ofs << static_cast<int>(_image[row][col]) << ' ';
    }
    ofs << '\n';
  }

  ofs.close();
}

} // namespace pgm_map_creator
