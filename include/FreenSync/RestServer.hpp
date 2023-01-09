#pragma once

#include <thread>
#include <optional>
#include <filesystem>

#include <restbed>

namespace FreenSync
{
  class FreenSyncCore;

  class RestServer;
  using SharedRestServer = std::shared_ptr<RestServer>;

  class RestServer
  {
    using SharedSession = std::shared_ptr<restbed::Session>;

  public:
    RestServer(FreenSyncCore* freenSyncCore);
    ~RestServer();

    bool start(int port);
    void stop();

  private:
    bool _stop();

    // Handlers
    void _getAvailableLights(const SharedSession& session) const;
    void _getSyncedLights(const SharedSession& session) const;
    void _getSyncedLight(const SharedSession& session) const;
    void _getAllLights(const SharedSession& session) const;
    void _getDisplayInfo(const SharedSession& session) const;
    void _getWebFile(const SharedSession& session) const;
    void _setLightUV(const SharedSession& session) const;
    void _setLightGammaFactor(const SharedSession& session) const;
    void _syncLight(const SharedSession& session) const;
    void _unsyncLight(const SharedSession& session) const;
    void _saveProfile(const SharedSession& session) const;

    // Attributes
    FreenSyncCore* m_freenSyncCore;

    std::shared_ptr<restbed::Settings> m_settings;
    std::optional<std::thread> m_serviceThread;
    restbed::Service m_service;

    const std::filesystem::path m_webroot;
    std::unordered_map<std::string, std::string> m_contentTypes;
  };
}
