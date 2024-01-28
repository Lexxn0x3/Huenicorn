#include <Huenicorn/Interpolation.hpp>


namespace Huenicorn
{
  namespace Interpolation
  {
    Interpolations availableInterpolations = {
      {"Nearest", Type::Nearest},
      {"Cubic", Type::Cubic},
      {"Area", Type::Area},
    };
  }
}
