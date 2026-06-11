#include "pgm_map_creator/geometry_utils.hh"

#include <limits>

#include <gz/sim/Util.hh>
#include <gz/sim/components/Collision.hh>
#include <gz/sim/components/Geometry.hh>
#include <gz/sim/components/ParentEntity.hh>

#include <sdf/Box.hh>
#include <sdf/Cylinder.hh>
#include <sdf/Mesh.hh>
#include <sdf/Sphere.hh>

namespace pgm_map_creator {

bool ComputeLocalBoundingBox(const sdf::Geometry &_geom,
                             gz::math::AxisAlignedBox &_box) {
  switch (_geom.Type()) {
  case sdf::GeometryType::BOX: {
    auto size = _geom.BoxShape()->Size();
    _box = gz::math::AxisAlignedBox(-size / 2.0, size / 2.0);
    return true;
  }
  case sdf::GeometryType::CYLINDER: {
    double r = _geom.CylinderShape()->Radius();
    double h = _geom.CylinderShape()->Length();
    _box = gz::math::AxisAlignedBox(gz::math::Vector3d(-r, -r, -h / 2.0),
                                    gz::math::Vector3d(r, r, h / 2.0));
    return true;
  }
  case sdf::GeometryType::SPHERE: {
    double r = _geom.SphereShape()->Radius();
    _box = gz::math::AxisAlignedBox(gz::math::Vector3d(-r, -r, -r),
                                    gz::math::Vector3d(r, r, r));
    return true;
  }
  case sdf::GeometryType::MESH: {
    auto scale = _geom.MeshShape()->Scale();
    _box = gz::math::AxisAlignedBox(-scale / 2.0, scale / 2.0);
    return true;
  }
  default:
    return false;
  }
}

std::vector<gz::math::Vector3d>
TransformBoxCorners(const gz::math::AxisAlignedBox &_box,
                    const gz::math::Pose3d &_pose) {
  std::vector<gz::math::Vector3d> corners;
  corners.reserve(8);

  auto mn = _box.Min();
  auto mx = _box.Max();

  corners.push_back(_pose.CoordPositionAdd({mn.X(), mn.Y(), mn.Z()}));
  corners.push_back(_pose.CoordPositionAdd({mn.X(), mn.Y(), mx.Z()}));
  corners.push_back(_pose.CoordPositionAdd({mn.X(), mx.Y(), mn.Z()}));
  corners.push_back(_pose.CoordPositionAdd({mn.X(), mx.Y(), mx.Z()}));
  corners.push_back(_pose.CoordPositionAdd({mx.X(), mn.Y(), mn.Z()}));
  corners.push_back(_pose.CoordPositionAdd({mx.X(), mn.Y(), mx.Z()}));
  corners.push_back(_pose.CoordPositionAdd({mx.X(), mx.Y(), mn.Z()}));
  corners.push_back(_pose.CoordPositionAdd({mx.X(), mx.Y(), mx.Z()}));

  return corners;
}

gz::math::AxisAlignedBox
ComputeWorldAABB(const std::vector<gz::math::Vector3d> &_corners) {
  gz::math::Vector3d minPt(std::numeric_limits<double>::max(),
                           std::numeric_limits<double>::max(),
                           std::numeric_limits<double>::max());
  gz::math::Vector3d maxPt(std::numeric_limits<double>::lowest(),
                           std::numeric_limits<double>::lowest(),
                           std::numeric_limits<double>::lowest());

  for (auto &c : _corners) {
    minPt.X(std::min(minPt.X(), c.X()));
    minPt.Y(std::min(minPt.Y(), c.Y()));
    minPt.Z(std::min(minPt.Z(), c.Z()));
    maxPt.X(std::max(maxPt.X(), c.X()));
    maxPt.Y(std::max(maxPt.Y(), c.Y()));
    maxPt.Z(std::max(maxPt.Z(), c.Z()));
  }

  return gz::math::AxisAlignedBox(minPt, maxPt);
}

void GatherCollisionBoxes(const gz::sim::EntityComponentManager &_ecm,
                          double _scanHeight,
                          std::vector<CollisionBox> &_boxes) {
  _ecm.Each<gz::sim::components::Collision, gz::sim::components::Geometry,
            gz::sim::components::ParentEntity>(
      [&](const gz::sim::Entity &_entity,
          const gz::sim::components::Collision *,
          const gz::sim::components::Geometry *_geom,
          const gz::sim::components::ParentEntity *) -> bool {
        const sdf::Geometry &geom = _geom->Data();
        gz::math::AxisAlignedBox localBox;

        if (!ComputeLocalBoundingBox(geom, localBox))
          return true; // skip unsupported types

        auto pose = gz::sim::worldPose(_entity, _ecm);
        auto corners = TransformBoxCorners(localBox, pose);
        auto worldBox = ComputeWorldAABB(corners);

        // Only include if collision overlaps the scan height range
        if (worldBox.Min().Z() < _scanHeight && worldBox.Max().Z() > 0.001) {
          _boxes.push_back({worldBox});
        }

        return true;
      });
}

bool CheckCollisionAtPoint(double _x, double _y, double _scanHeight,
                           const std::vector<CollisionBox> &_boxes) {
  for (auto &cb : _boxes) {
    auto &box = cb.box;
    if (_x >= box.Min().X() && _x <= box.Max().X() && _y >= box.Min().Y() &&
        _y <= box.Max().Y()) {
      if (box.Min().Z() < _scanHeight && box.Max().Z() > 0.001) {
        return true;
      }
    }
  }
  return false;
}

} // namespace pgm_map_creator
