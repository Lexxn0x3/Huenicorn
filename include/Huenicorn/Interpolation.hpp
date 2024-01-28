#pragma once

#include <unordered_map>
#include <string>


namespace Huenicorn
{
  namespace Interpolation
  {
    enum class Type
    {
      Linear = 0,
      Nearest = 1,
      Area = 2
    };


    using Interpolations = std::unordered_map<std::string, Type>;

    extern Interpolations availableInterpolations;
  }
}
