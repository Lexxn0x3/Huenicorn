#pragma once
#include <array>

#include <glm/vec2.hpp>
#include <glm/geometric.hpp>


namespace Huenicorn
{
  namespace ColorUtils
  {
    using GamutCoordinates = std::array<glm::vec2, 3>;


    // Wide RGB D65 factors
    static constexpr glm::mat3x3 D65Factors = {
      {0.649926f, 0.103455f, 0.197109f},
      {0.234327f, 0.743075f, 0.022598f},
      {0.000000f, 0.053077f, 1.035763f}
    };


    /**
     * @brief Computes a sign check on boundaries
     * 
     * @param a Gamut vertex a
     * @param b Gamut vertex b
     * @param c Gamut vertex c
     * @return float Signed value for boundary check
     */
    static inline float sign(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c)
    {
      return (a.x - c.x) * (b.y - c.y) - (b.x - c.x) * (a.y - c.y);
    }


    /**
     * @brief Returns whether the XY color coordinates fits in gamut
     * 
     * @param xy 
     * @param gamutCoordinates 
     * @return true XY color fits in gamut boundaries
     * @return false XY color doesn't fit in gamut boundaries
     */
    static inline bool xyInGamut(const glm::vec2& xy, const ColorUtils::GamutCoordinates& gamutCoordinates)
    {
      bool has_neg, has_pos;

      const auto& a = gamutCoordinates.at(0);
      const auto& b = gamutCoordinates.at(1);
      const auto& c = gamutCoordinates.at(2);

      float d1 = sign(xy, a, b);
      float d2 = sign(xy, b, c);
      float d3 = sign(xy, c, a);

      has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
      has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

      return !(has_neg && has_pos);
    }


    /**
     * @brief 
     * 
     * @param xy Input coordinates to clamp
     * @param gamut Coordinates of the gamut triangle
     * @return glm::vec2 Clamped xy coordinates
     */
    static glm::vec2 clampToGamut(const glm::vec2& xy, const GamutCoordinates& gamut)
    {
      // https://stackoverflow.com/questions/2924795/fastest-way-to-compute-point-to-triangle-distance-in-3d
      const glm::vec2& p = xy;
      const glm::vec2& a = gamut[0];
      const glm::vec2& b = gamut[1];
      const glm::vec2& c = gamut[2];

      glm::vec2 ab = b - a;
      glm::vec2 ac = c - a;
      glm::vec2 ap = p - a;

      float d1 = glm::dot(ab, ap);
      float d2 = glm::dot(ac, ap);
      if(d1 <= 0 && d2 <= 0){
        return a;
      }

      glm::vec2 bp = p - b;
      float d3 = glm::dot(ab, bp);
      float d4 = glm::dot(ac, bp);
      if (d3 >= 0 && d4 <= d3){
        return b;
      }

      glm::vec2 cp = p - c;
      float d5 = glm::dot(ab, cp);
      float d6 = glm::dot(ac, cp);
      if (d6 >= 0 && d5 <= d6){
        return c;
      }

      float vc = d1 * d4 - d3 * d2;
      if (vc <= 0 && d1 >= 0 && d3 <= 0){
        float v = d1 / (d1 - d3);
        return a + v * ab;
      }

      float vb = d5 * d2 - d1 * d6;
      if (vb <= 0 && d2 >= 0 && d6 <= 0){
        float v = d2 / (d2 - d6);
        return a + v * ac;
      }

      float va = d3 * d6 - d5 * d4;
      if (va <= 0 && (d4 - d3) >= 0 && (d5 - d6) >= 0){
        float v = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        return b + v * (c - b);
      }

      float denom = 1.0 / (va + vb + vc);
      float v = vb * denom;
      float w = vc * denom;

      return a + v * ab + w * ac;
    }
  }
}
