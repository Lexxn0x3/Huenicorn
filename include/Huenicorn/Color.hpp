#pragma once

#include <cstdint>

#include <array>
#include <optional>

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
//#include <glm/gtx/string_cast.hpp>

#include <Huenicorn/ColorUtils.hpp>

namespace Huenicorn
{
  /**
   * @brief Color data structure providing conversion and manipulation methods
   * 
   */
  class Color
  {
  public:
    using ChannelDepth = uint8_t;
    static constexpr float Max = static_cast<float>(std::numeric_limits<ChannelDepth>().max());


    /**
     * @brief Color constructor
     * 
     * @param r Red channel
     * @param g Green channel
     * @param b Blue channel
     */
    Color(ChannelDepth r = 0, ChannelDepth g = 0,  ChannelDepth b = 0):
    m_r(r),
    m_g(g),
    m_b(b)
    {}

    /**
     * @brief Equality comparison operator
     * 
     * @param other Other color to compare
     * @return true Other color is equal
     * @return false Other color is not equal
     */
    bool operator==(const Color& other) const
    {
      return  m_r == other.m_r &&
              m_g == other.m_g &&
              m_b == other.m_b;
    }

    /**
     * @brief Inequality comparison operator
     * 
     * @param other Other color to compare
     * @return true Other color is not equal
     * @return false Other color is equal
     */
    bool operator!=(const Color& other) const
    {
      return  !(*this == other);
    }


    /**
     * @brief Returns a rgb value as normalized 0-1 floating range
     * 
     * @return glm::vec3 RGB color as 0-1 floating range
     */
    glm::vec3 asRGB() const
    {
      return glm::vec3(
        m_r / Color::Max,
        m_g / Color::Max,
        m_b / Color::Max
      );
    }


    /**
     * @brief Returns a XYZ conversion of RGB color
     * 
     * @param gamutCoordinates Optional boundaries of the gamut. Discard check/clamp if not provided
     * @return glm::vec3 XYZ color coordinates
     */
    glm::vec3 asXYZ(const std::optional<ColorUtils::GamutCoordinates>& gamutCoordinates = std::nullopt) const
    {
      // Following https://gist.github.com/popcorn245/30afa0f98eea1c2fd34d
      glm::vec3 normalizedRgb = this->asRGB();

      // Apply gamma
      for(int i = 0; i < normalizedRgb.length(); i++){
        _applyGamma(normalizedRgb[i]);
      }

      // Apply some magic "Wide RGB D65 conversion formula"
      float& r = normalizedRgb.r;
      float& g = normalizedRgb.g;
      float& b = normalizedRgb.b;

      float x = r * 0.649926f + g * 0.103455f + b * 0.197109f;
      float y = r * 0.234327f + g * 0.743075f + b * 0.022598f;
      float z = r * 0.000000f + g * 0.053077f + b * 1.035763f;


      // Black coordinates to be neutral in case of black (skip dividing by zero)
      glm::vec3 xyz{0.31273010828044345f, 0.3290198826715099f, 0.0f};

      float sum = x + y + z;
      if(sum != 0.f){
        xyz[0] = x / sum;
        xyz[1] = y / sum;
        xyz[2] = y;
      }

      glm::vec2 xy = {xyz.x, xyz.y};
      if(gamutCoordinates.has_value() && !ColorUtils::xyInGamut(xy, gamutCoordinates.value())){
        glm::vec2 xyClamped = ColorUtils::clampToGamut(xy, gamutCoordinates.value());
        xyz = glm::vec3{xyClamped.x, xyClamped.y, xyz.z};
      }

      return xyz;
    }


    /**
     * @brief Returns the ponderated brightness of RGB color
     * 
     * @return float Color brightness
     */
    float brightness() const
    {
      return (m_r * 0.3f + m_g * 0.59f + m_b * 0.11f) / Color::Max;
    }


  /**
   * @brief Applies selective gamma to input value
   * 
     * @param channel Channel value to alterate
   * 
   */
  private:
    static void _applyGamma(float& channel)
    {
      channel = (channel > 0.04045f) ? pow((channel + 0.055f) / (1.0f + 0.055f), 2.4f) : (channel / 12.92f);
    }


  private:
    // Attributes
    ChannelDepth m_r;
    ChannelDepth m_g;
    ChannelDepth m_b;
  };
}
