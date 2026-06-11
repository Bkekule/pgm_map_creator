#include <chrono>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <string>
#include <thread>

#include <gz/msgs/vector2d.pb.h>
#include <gz/transport/Node.hh>

#include "pgm_map_creator/vector_parsing.hh"

#include "collision_map_request.pb.h"

int main(int argc, char *argv[]) {
  if (argc < 5) {
    std::cerr << "Usage: request_publisher "
              << "'(x1,y1)(x2,y2)(x3,y3)(x4,y4)' "
              << "height resolution filename [threshold]" << std::endl;
    return -1;
  }

  collision_map_creator_msgs::msgs::CollisionMapRequest request;
  std::deque<gz::msgs::Vector2d *> corners;

  corners.push_back(request.mutable_upper_left());
  corners.push_back(request.mutable_upper_right());
  corners.push_back(request.mutable_lower_right());
  corners.push_back(request.mutable_lower_left());

  if (!pgm_map_creator::ParseVectorArray(argv[1], corners))
    return -1;

  request.set_height(std::atof(argv[2]));
  request.set_resolution(std::atof(argv[3]));
  request.set_filename(argv[4]);
  request.set_threshold(argc >= 6 ? std::atoi(argv[5]) : 255);

  // Publish via gz-transport
  gz::transport::Node node;
  auto pub =
      node.Advertise<collision_map_creator_msgs::msgs::CollisionMapRequest>(
          "/collision_map/command");

  std::cout << "Waiting for subscriber on /collision_map/command ..."
            << std::endl;

  std::this_thread::sleep_for(std::chrono::seconds(2));

  while (!pub.HasConnections())
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

  std::cout << "Publishing request:"
            << " UL(" << request.upper_left().x() << ","
            << request.upper_left().y() << ")"
            << " UR(" << request.upper_right().x() << ","
            << request.upper_right().y() << ")"
            << " LR(" << request.lower_right().x() << ","
            << request.lower_right().y() << ")"
            << " LL(" << request.lower_left().x() << ","
            << request.lower_left().y() << ")"
            << " H=" << request.height() << " Res=" << request.resolution()
            << " File=" << request.filename() << std::endl;

  pub.Publish(request);
  std::cout << "Done." << std::endl;

  std::this_thread::sleep_for(std::chrono::seconds(1));
  return 0;
}
