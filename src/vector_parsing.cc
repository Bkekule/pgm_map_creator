#include "pgm_map_creator/vector_parsing.hh"

#include <cstdlib>
#include <iostream>
#include <string>

namespace pgm_map_creator {

bool ParseVectorArray(const char *_vectorString,
                      std::deque<gz::msgs::Vector2d *> _corners) {
  std::string cornersStr = _vectorString;
  size_t opening = 0;
  size_t closing = 0;

  for (auto it = _corners.begin(); it != _corners.end(); ++it) {
    opening = cornersStr.find('(', closing);
    closing = cornersStr.find(')', opening);
    if (opening == std::string::npos || closing == std::string::npos) {
      std::cerr << "Poorly formed string: " << cornersStr << std::endl;
      std::cerr << "( found at: " << opening << " ) found at: " << closing
                << std::endl;
      return false;
    }

    std::string oneCorner =
        cornersStr.substr(opening + 1, closing - opening - 1);
    size_t commaLoc = oneCorner.find(',');
    std::string x = oneCorner.substr(0, commaLoc);
    std::string y = oneCorner.substr(commaLoc + 1);

    (*it)->set_x(std::atof(x.c_str()));
    (*it)->set_y(std::atof(y.c_str()));
  }
  return true;
}

} // namespace pgm_map_creator
