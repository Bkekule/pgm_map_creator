#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include <gz/plugin/Register.hh>
#include <gz/sim/EntityComponentManager.hh>
#include <gz/sim/System.hh>
#include <gz/transport/Node.hh>

#include "pgm_map_creator/geometry_utils.hh"
#include "pgm_map_creator/pgm_writer.hh"

#include "collision_map_request.pb.h"

namespace pgm_map_creator {

/// \brief Gz Sim system plugin that generates a PGM occupancy map.
///
/// Subscribes to `/collision_map/command` and on the next simulation step
/// rasterizes world collision geometries into a 2D grid, then writes a PGM
/// file.
class CollisionMapCreator : public gz::sim::System,
                            public gz::sim::ISystemConfigure,
                            public gz::sim::ISystemPostUpdate {
public:
  // -- ISystemConfigure --
  void Configure(const gz::sim::Entity &_entity,
                 const std::shared_ptr<const sdf::Element> & /*_sdf*/,
                 gz::sim::EntityComponentManager & /*_ecm*/,
                 gz::sim::EventManager & /*_eventMgr*/) override {
    this->worldEntity = _entity;

    this->node.Subscribe("/collision_map/command",
                         &CollisionMapCreator::OnRequest, this);

    std::cout << "[CollisionMapCreator] Plugin loaded. "
              << "Listening on /collision_map/command" << std::endl;
  }

  // -- ISystemPostUpdate --
  void PostUpdate(const gz::sim::UpdateInfo & /*_info*/,
                  const gz::sim::EntityComponentManager &_ecm) override {
    if (!this->requestPending)
      return;

    this->requestPending = false;
    this->ProcessRequest(_ecm);
  }

private:
  void
  OnRequest(const collision_map_creator_msgs::msgs::CollisionMapRequest &_msg) {
    std::cout << "[CollisionMapCreator] Received map request" << std::endl;
    this->pendingMsg = _msg;
    this->requestPending = true;
  }

  void ProcessRequest(const gz::sim::EntityComponentManager &_ecm) {
    const auto &msg = this->pendingMsg;

    std::cout << "[CollisionMapCreator] Map bounds: (" << msg.upper_left().x()
              << "," << msg.upper_left().y() << ") to ("
              << msg.lower_right().x() << "," << msg.lower_right().y() << ")\n"
              << "  Height=" << msg.height()
              << "  Resolution=" << msg.resolution() << " m/px\n"
              << "  Threshold=" << msg.threshold() << std::endl;

    // 1. Gather collision boxes from the world
    double scanHeight = msg.height();
    std::vector<CollisionBox> collisionBoxes;
    GatherCollisionBoxes(_ecm, scanHeight, collisionBoxes);

    std::cout << "[CollisionMapCreator] Found " << collisionBoxes.size()
              << " collision geometries" << std::endl;

    // 2. Compute rasterization grid
    double dXv = msg.upper_left().x() - msg.lower_left().x();
    double dYv = msg.upper_left().y() - msg.lower_left().y();
    double magV = std::sqrt(dXv * dXv + dYv * dYv);
    dXv = msg.resolution() * dXv / magV;
    dYv = msg.resolution() * dYv / magV;

    double dXh = msg.upper_right().x() - msg.upper_left().x();
    double dYh = msg.upper_right().y() - msg.upper_left().y();
    double magH = std::sqrt(dXh * dXh + dYh * dYh);
    dXh = msg.resolution() * dXh / magH;
    dYh = msg.resolution() * dYh / magH;

    int rows = static_cast<int>(magV / msg.resolution());
    int cols = static_cast<int>(magH / msg.resolution());

    if (rows == 0 || cols == 0) {
      std::cerr << "[CollisionMapCreator] Zero-dimension image. "
                << "Check coordinates." << std::endl;
      return;
    }

    // 3. Rasterize
    int threshold = msg.threshold() > 0 ? msg.threshold() : 255;
    uint8_t fillVal = static_cast<uint8_t>(255 - threshold);
    uint8_t blankVal = 255;

    std::vector<std::vector<uint8_t>> image(
        rows, std::vector<uint8_t>(cols, blankVal));

    std::cout << "[CollisionMapCreator] Rasterizing " << cols << "x" << rows
              << " grid ..." << std::endl;

    for (int i = 0; i < rows; ++i) {
      if (i % 100 == 0)
        std::cout << "  " << (i * 100 / rows) << "% complete" << std::endl;

      double baseX = i * dXv + msg.lower_left().x();
      double baseY = i * dYv + msg.lower_left().y();

      for (int j = 0; j < cols; ++j) {
        double x = baseX + j * dXh;
        double y = baseY + j * dYh;

        if (CheckCollisionAtPoint(x, y, scanHeight, collisionBoxes))
          image[i][j] = fillVal;
      }
    }

    // 4. Write output
    if (!msg.filename().empty()) {
      WritePgm(msg.filename(), image, cols, rows);
      std::cout << "[CollisionMapCreator] Wrote " << msg.filename() << ".pgm"
                << std::endl;
    }
  }

  gz::sim::Entity worldEntity;
  gz::transport::Node node;
  collision_map_creator_msgs::msgs::CollisionMapRequest pendingMsg;
  bool requestPending{false};
};

} // namespace pgm_map_creator

GZ_ADD_PLUGIN(pgm_map_creator::CollisionMapCreator, gz::sim::System,
              pgm_map_creator::CollisionMapCreator::ISystemConfigure,
              pgm_map_creator::CollisionMapCreator::ISystemPostUpdate)

GZ_ADD_PLUGIN_ALIAS(pgm_map_creator::CollisionMapCreator,
                    "pgm_map_creator::CollisionMapCreator")
