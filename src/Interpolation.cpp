#include <Huenicorn/Interpolation.hpp>


namespace Huenicorn
{
  namespace Interpolation
  {
    Interpolations availableInterpolations = {
      {"Linear", Type::Linear},
      {"Nearest", Type::Nearest},
      {"Area", Type::Area},
    };
  }
}
