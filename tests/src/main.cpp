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
  Color::GamutCoordinates gammutCoordinates = {coordinate, coordinate, coordinate};

  cout << "black : " << to_string(black.asRGB()) << endl;
  cout << "        " << to_string(black.asXYZ(gammutCoordinates)) << endl;
  cout << "white : " << to_string(white.asRGB()) << endl;
  cout << "        " << to_string(white.asXYZ(gammutCoordinates)) << endl;


  cout << "Tests" << endl;
  return 0;
}
