#include <iostream>

#include <glm/gtx/string_cast.hpp>

#include <Huenicorn/ColorUtils.hpp>
#include <Huenicorn/Color.hpp>

#include <nlohmann/json.hpp>

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

using namespace glm;
using namespace nlohmann;
using namespace std;
using namespace Huenicorn;

TEST_CASE("Color conversions are computed", "[color conversions]")
{
  ColorUtils::GamutCoordinates gamutCoordinates = {
    vec2(0.6915, 0.3083),
    vec2(0.17, 0.7),
    vec2(0.1532, 0.0475)
  };

  float threshold = std::numeric_limits<float>::epsilon();

  // Inputs
  Color white(255, 255, 255);
  Color black(0, 0, 0);
  Color r(255, 0, 0);
  Color g(0, 255, 0);
  Color b(0, 0, 255);
  Color c(0, 255, 255);
  Color m(255, 0, 255);
  Color y(255, 255, 0);

  // Control samples (Computed from web applet)
  vec3 vec3White{0.3127301082804434, 0.3290198826715099, 1};
  vec3 vec3Black{0.3127301082804434, 0.3290198826715099, 0};
  vec3 vec3R{0.6915, 0.3083, 0.234327};
  vec3 vec3G{0.17, 0.7, 0.743075};
  vec3 vec3B{0.1532, 0.0475, 0.022598};
  vec3 vec3C{0.16111030949842242, 0.3547307706976564, 0.765673};
  vec3 vec3M{0.3782074734869837, 0.1565134666271695, 0.256925};
  vec3 vec3Y{0.4043527457683985, 0.523977045987571, 0.9774020000000001};

  // Testing RGB conversions
  REQUIRE(glm::distance(black.asRGB(), glm::vec3(0, 0, 0)) <= threshold);
  REQUIRE(glm::distance(white.asRGB(), glm::vec3(1, 1, 1)) <= threshold);
  REQUIRE(glm::distance(r.asRGB(), glm::vec3(1, 0, 0)) <= threshold);
  REQUIRE(glm::distance(g.asRGB(), glm::vec3(0, 1, 0)) <= threshold);
  REQUIRE(glm::distance(b.asRGB(), glm::vec3(0, 0, 1)) <= threshold);
  REQUIRE(glm::distance(c.asRGB(), glm::vec3(0, 1, 1)) <= threshold);
  REQUIRE(glm::distance(m.asRGB(), glm::vec3(1, 0, 1)) <= threshold);
  REQUIRE(glm::distance(y.asRGB(), glm::vec3(1, 1, 0)) <= threshold);

  // Testing XYZ conversions
  REQUIRE(glm::distance(black.asXYZ(gamutCoordinates), vec3Black) <= threshold);
  REQUIRE(glm::distance(white.asXYZ(gamutCoordinates), vec3White) <= threshold);
  REQUIRE(glm::distance(r.asXYZ(gamutCoordinates), vec3R) <= threshold);
  REQUIRE(glm::distance(g.asXYZ(gamutCoordinates), vec3G) <= threshold);
  REQUIRE(glm::distance(b.asXYZ(gamutCoordinates), vec3B) <= threshold);
  REQUIRE(glm::distance(c.asXYZ(gamutCoordinates), vec3C) <= threshold);
  REQUIRE(glm::distance(m.asXYZ(gamutCoordinates), vec3M) <= threshold);
  REQUIRE(glm::distance(y.asXYZ(gamutCoordinates), vec3Y) <= threshold);
}
