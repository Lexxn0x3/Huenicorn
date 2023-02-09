#pragma once

#include <optional>
#include <thread>

#include <Huenicorn/IGrabber.hpp>
#include <Huenicorn/IRestServer.hpp>
#include <Huenicorn/BridgeData.hpp>
#include <Huenicorn/TickSynchronizer.hpp>


namespace Huenicorn
{
  using SyncedLights = std::unordered_map<std::string, SharedSyncedLight>;
  using SharedBridgeData = std::shared_ptr<BridgeData>;

  class HuenicornCore
  {
    struct ThreadedRestService
    {
      std::unique_ptr<IRestServer> server;
      std::optional<std::thread> thread;
    };

  public:
    // Constructor
    HuenicornCore(const std::filesystem::path& configRoot);

    // Getters
    const std::filesystem::path configFilePath() const;
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
    nlohmann::json autoDetectedBridge() const;
    nlohmann::json requestNewApiKey();


    // Setters
    const SyncedLight::UVs& setLightUV(const std::string& syncedLightId, SyncedLight::UV&& uv, SyncedLight::UVType uvType);
    void setLightGammaFactor(const std::string& syncedLightId, float gammaFactor);
    void setSubsampleWidth(unsigned subsampleWidth);
    void setRefreshRate(unsigned subsampleWidth);
    void setTransitionTime_c(unsigned transitionTime_c);

    // Methods
    void start();
    void stop();
    bool validateBridgeAddress(const std::string& bridgeAddress);
    bool validateApiKey(const std::string& apiKey);
    SharedSyncedLight addSyncedLight(const std::string& lightId);
    bool removeSyncedLight(const std::string& lightId);
    void saveProfile() const;


  private:

    // Private methods
    bool _loadProfile();
    void _spawnBrowser();
    void _loop();
    void _processScreenFrame();
    void _shutdownLights();
    void _resetJsonLightsCache();


    // Attributes
    std::filesystem::path m_configRoot;
    std::filesystem::path m_profileFilePath;
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

    // Service and flags
    bool m_openedSetup{false};
    ThreadedRestService m_webUIService;

    //  Image Processing
    std::unique_ptr<IGrabber> m_grabber;
    cv::Mat m_cvImage;
  };
}
