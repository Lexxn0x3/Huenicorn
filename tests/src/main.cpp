#include <iostream>

#include <glm/gtx/string_cast.hpp>

#include <Huenicorn/Color.hpp>


using namespace glm;
using namespace std;
using namespace Huenicorn;

int main()
{
  auto max = std::numeric_limits<Color::ChannelDepth>::max();


  Color black{0, 0, 0};
  Color white{max, max, max};

  vec2 coordinate{0, 0};
  Color::GamutCoordinates gamutCoordinates = {coordinate, coordinate, coordinate};

  cout << "black : " << to_string(black.asRGB()) << endl;
  cout << "        " << to_string(black.asXYZ(gamutCoordinates)) << endl;
  cout << "white : " << to_string(white.asRGB()) << endl;
  cout << "        " << to_string(white.asXYZ(gamutCoordinates)) << endl;


  cout << "Tests" << endl;
  return 0;
}
