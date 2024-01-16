#include <Huenicorn/Channel.hpp>

#include <glm/common.hpp>


namespace Huenicorn
{
  Channel::Channel(bool active, const std::vector<Device>& devices, float gammaFactor, const ColorUtils::GamutCoordinates& gamutCoordinates, const UVs& uvs):
  m_state(active ? State::Active : State::Inactive),
  m_devices(devices),
  m_gamutCoordinates(gamutCoordinates),
  m_gammaFactor(gammaFactor),
  m_uvs(uvs)
  {}


  Channel::State Channel::state() const
  {
    return m_state;
  }


  const UVs& Channel::uvs() const
  {
    return m_uvs;
  }
  
  
  float Channel::gammaFactor() const
  {
    return m_gammaFactor;
  }


  const std::vector<Device>& Channel::devices() const
  {
    return m_devices;
  }


  void Channel::setActive(bool active)
  {
    if(active){
      m_state = State::Active;
    }
    else{
      m_state = State::PendingShutdown;
    }
  }


  UVs& Channel::setUV(UV&& uv, UVCorner uvCorner)
  {
    UVs newUVs = m_uvs;
    uv.x = glm::clamp(uv.x, 0.f, 1.f);
    uv.y = glm::clamp(uv.y, 0.f, 1.f);

    switch (uvCorner)
    {
      case UVCorner::TopLeft:
      {
        newUVs.min.x = uv.x;
        newUVs.min.y = uv.y;
        newUVs.max.x = glm::max(uv.x, newUVs.max.x);
        newUVs.max.y = glm::max(uv.y, newUVs.max.y);
        break;
      }

      case UVCorner::TopRight:
      {
        newUVs.max.x = uv.x;
        newUVs.min.y = uv.y;
        newUVs.min.x = glm::min(uv.x, newUVs.min.x);
        newUVs.max.y = glm::max(uv.y, newUVs.max.y);
        break;
      }

      case UVCorner::BottomLeft:
      {
        newUVs.min.x = uv.x;
        newUVs.max.y = uv.y;
        newUVs.max.x = glm::max(uv.x, newUVs.max.x);
        newUVs.min.y = glm::min(uv.y, newUVs.min.y);
        break;
      }

      case UVCorner::BottomRight:
      {
        newUVs.max.x = uv.x;
        newUVs.max.y = uv.y;
        newUVs.min.x = glm::min(uv.x, newUVs.min.x);
        newUVs.min.y = glm::min(uv.y, newUVs.min.y);
        break;
      }

      default:
        break;
    }

    std::swap(m_uvs, newUVs);

    return m_uvs;
  }


  void Channel::setGammaFactor(float gammaFactor)
  {
    m_gammaFactor = gammaFactor;
  }


  void Channel::acknowledgeShutdown()
  {
    m_state = State::Inactive;
  }
}
