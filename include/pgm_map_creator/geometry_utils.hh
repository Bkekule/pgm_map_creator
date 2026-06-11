#ifndef PGM_MAP_CREATOR_GEOMETRY_UTILS_HH
#define PGM_MAP_CREATOR_GEOMETRY_UTILS_HH

#include <vector>

#include <gz/math/AxisAlignedBox.hh>
#include <gz/math/Pose3.hh>
#include <gz/math/Vector3.hh>
#include <gz/sim/EntityComponentManager.hh>

#include <sdf/Geometry.hh>

namespace pgm_map_creator {

/// \brief Axis-aligned bounding box in world frame for a collision entity
struct CollisionBox {
  gz::math::AxisAlignedBox box;
};

/// \brief Compute a local AABB from an SDF geometry element.
/// \param[in] _geom The SDF geometry to compute bounds for
/// \param[out] _box The resulting local-frame bounding box
/// \return True if the geometry type is supported
bool ComputeLocalBoundingBox(const sdf::Geometry &_geom,
                             gz::math::AxisAlignedBox &_box);

/// \brief Transform a local AABB by a pose, returning the 8 world-frame
/// corners.
/// \param[in] _box Local axis-aligned bounding box
/// \param[in] _pose World pose to apply
/// \return Vector of 8 transformed corner points
std::vector<gz::math::Vector3d>
TransformBoxCorners(const gz::math::AxisAlignedBox &_box,
                    const gz::math::Pose3d &_pose);

/// \brief Compute the world-frame AABB from transformed corners.
/// \param[in] _corners The 8 world-frame corner points
/// \return Axis-aligned bounding box enclosing all corners
gz::math::AxisAlignedBox
ComputeWorldAABB(const std::vector<gz::math::Vector3d> &_corners);

/// \brief Gather all collision bounding boxes from the ECM that overlap
///        the scan height range [0, scanHeight].
/// \param[in] _ecm Entity component manager
/// \param[in] _scanHeight Maximum Z height for inclusion
/// \param[out] _boxes Output vector of world-frame collision boxes
void GatherCollisionBoxes(const gz::sim::EntityComponentManager &_ecm,
                          double _scanHeight,
                          std::vector<CollisionBox> &_boxes);

/// \brief Check if a vertical ray at (x, y) from Z=0 to Z=scanHeight
///        intersects any of the given collision bounding boxes.
/// \param[in] _x X coordinate of the ray
/// \param[in] _y Y coordinate of the ray
/// \param[in] _scanHeight Max Z of the vertical ray
/// \param[in] _boxes Collision boxes to test against
/// \return True if the point is occupied
bool CheckCollisionAtPoint(double _x, double _y, double _scanHeight,
                           const std::vector<CollisionBox> &_boxes);

} // namespace pgm_map_creator

#endif // PGM_MAP_CREATOR_GEOMETRY_UTILS_HH
