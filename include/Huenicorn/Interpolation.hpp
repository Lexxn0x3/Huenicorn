#pragma once

#include <unordered_map>
#include <string>


namespace Huenicorn
{
  namespace Interpolation
  {
    enum class Type
    {
      Nearest = 0,
      Cubic = 1,
      Area = 2
    };


    using Interpolations = std::unordered_map<std::string, Type>;

    extern Interpolations availableInterpolations;
  }
}
