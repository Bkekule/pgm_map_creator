#ifndef PGM_MAP_CREATOR_VECTOR_PARSING_HH
#define PGM_MAP_CREATOR_VECTOR_PARSING_HH

#include <deque>

#include "collision_map_request.pb.h"

namespace pgm_map_creator {

/// \brief Parse a string of the form "(x1,y1)(x2,y2)..." into Vector2d
/// messages.
/// \param[in] _vectorString The input string with parenthesized coordinate
/// pairs
/// \param[in,out] _corners Deque of pointers to Vector2d messages to populate
/// \return True on success, false if the string is malformed
bool ParseVectorArray(
    const char *_vectorString,
    std::deque<collision_map_creator_msgs::msgs::Vector2d *> _corners);

} // namespace pgm_map_creator

#endif // PGM_MAP_CREATOR_VECTOR_PARSING_HH
