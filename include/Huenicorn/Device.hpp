#pragma once

#include <string>
#include <unordered_map>
//#include <array>

//#include <glm/vec2.hpp>

namespace Huenicorn
{
  //using GamutCoordinates = std::array<glm::vec2, 3>;
  /**
   * @brief Device data structure
   * 
   */
  struct Device
  {
    std::string id;
    std::string name;
    std::string type;
    // GamutCoordinates gamutCoordinates;// ToDo
  };

  using Devices = std::unordered_map<std::string, Device>;
}
