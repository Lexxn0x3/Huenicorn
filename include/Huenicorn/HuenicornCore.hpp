#pragma once

#include <optional>
#include <thread>

#include <Huenicorn/ImageProcessing.hpp>
#include <Huenicorn/BridgeData.hpp>
#include <Huenicorn/ImageData.hpp>
#include <Huenicorn/RestServer.hpp>
#include <Huenicorn/TickSynchronizer.hpp>


namespace Huenicorn
{
  using SyncedLights = std::unordered_map<std::string, SharedSyncedLight>;
  using SharedBridgeData = std::shared_ptr<BridgeData>;

  class HuenicornCore
  {
  public:
    // Constructor
    HuenicornCore();


    // Getters
    const LightSummaries& availableLights() const;
    const SyncedLights& syncedLights() const;
    SharedSyncedLight syncedLight(const std::string& lightId) const;
    const nlohmann::json& jsonAvailableLights() const;
    const nlohmann::json& jsonSyncedLights() const;
    const nlohmann::json& jsonAllLights() const;
    bool syncedLightExists(const std::string& lightId) const;
    glm::ivec2 screenResolution() const;
    std::vector<glm::ivec2> subsampleResolutionCandidates() const;
    unsigned subsampleWidth() const;
    unsigned refreshRate() const;
    unsigned transitionTime_c() const;


    // Setters
    const SyncedLight::UVs& setLightUV(const std::string& syncedLightId, SyncedLight::UV&& uv, SyncedLight::UVType uvType);
    void setLightGammaFactor(const std::string& syncedLightId, float gammaFactor);
    void setSubsampleWidth(unsigned subsampleWidth);
    void setRefreshRate(unsigned subsampleWidth);
    void setTransitionTime_c(unsigned transitionTime_c);

    // Methods
    void start();
    void stop();
    SharedSyncedLight addSyncedLight(const std::string& lightId);
    bool removeSyncedLight(const std::string& lightId);
    void saveProfile() const;


  private:

    // Private methods
    bool _registerBridgeAddress();
    bool _registerApiToken();

    void _loadProfile();
    void _loop();
    void _processScreenFrame();
    void _shutdownLights();
    void _resetJsonLightsCache();


    // Attributes
    Config m_config;

    bool m_keepLooping;
    std::unique_ptr<TickSynchronizer> m_tickSynchronizer;

    //  Infrastructure
    SharedBridgeData m_bridge;
    SyncedLights m_syncedLights;

    // Cache
    mutable std::optional<nlohmann::json> m_cachedJsonAvailableLights;
    mutable std::optional<nlohmann::json> m_cachedJsonSyncedLights;
    mutable std::optional<nlohmann::json> m_cachedJsonAllLights;

    //  Image Processing
    ImageData m_imageData;


    //  Rest server
    RestServer m_restServer;
  };
}
