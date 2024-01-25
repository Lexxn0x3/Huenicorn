#include <Huenicorn/Config.hpp>

#include <filesystem>
#include <fstream>

#include <Huenicorn/JsonSerializer.hpp>
#include <Huenicorn/Logger.hpp>


namespace Huenicorn
{
  Config::Config(const std::filesystem::path& settingsRoot):
  m_configFilePath(settingsRoot / "config.json")
  {
    _loadConfigFile();
  }


  const std::filesystem::path& Config::configFilePath() const
  {
    return m_configFilePath;
  }


  bool Config::initialSetupOk() const
  {
    return m_bridgeAddress.has_value() && m_credentials.has_value();
  }


  int Config::restServerPort() const
  {
    return m_restServerPort;
  }


  const std::string& Config::boundBackendIP() const
  {
    return m_boundBackendIP;
  }


  unsigned Config::refreshRate() const
  {
    return m_refreshRate;
  }


  unsigned Config::subsampleWidth() const
  {
    return m_subsampleWidth;
  }


  const std::optional<std::string>& Config::bridgeAddress() const
  {
    return m_bridgeAddress;
  }


  const std::string& Config::username() const
  {
    return m_credentials.value().username();
  }


  const std::string& Config::clientkey() const
  {
    return m_credentials.value().clientkey();
  }


  const std::optional<std::string>& Config::profileName() const
  {
    return m_profileName;
  }


  const std::optional<Credentials>& Config::credentials() const
  {
    return m_credentials;
  }


  void Config::setBridgeAddress(const std::string& bridgeAddress)
  {
    m_bridgeAddress.emplace(bridgeAddress);
    _save();
  }


  void Config::setCredentials(const std::string& username, const std::string& clientkey)
  {
    m_credentials.emplace(username, clientkey);
    _save();
  }


  void Config::setProfileName(const std::string& profileName)
  {
    m_profileName.emplace(profileName);
    _save();
  }


  void Config::setSubsampleWidth(unsigned subsampleWidth)
  {
    m_subsampleWidth = subsampleWidth;
    _save();
  }


  void Config::setRefreshRate(unsigned refreshRate)
  {
    if(refreshRate < 1){
      refreshRate = 1;
    }

    m_refreshRate = refreshRate;
    _save();
  }


  bool Config::_loadConfigFile()
  {
    nlohmann::json jsonConfigRoot = nlohmann::json::object();
    bool requireSave = false;

    if(std::filesystem::exists(m_configFilePath)){
      jsonConfigRoot = nlohmann::json::parse(std::ifstream(m_configFilePath));
    }

    if(jsonConfigRoot.contains("restServerPort")){
      m_restServerPort = jsonConfigRoot.at("restServerPort");
    }
    else{
      requireSave = true;
    }

    if(jsonConfigRoot.contains("boundBackendIP")){
      m_boundBackendIP = jsonConfigRoot.at("boundBackendIP");
    }
    else{
      requireSave = true;
    }

    bool ready = true;
    if(!jsonConfigRoot.contains("bridgeAddress")){
      ready = false;
    }
    else{
      m_bridgeAddress.emplace(jsonConfigRoot.at("bridgeAddress"));
    }

    if(!jsonConfigRoot.contains("credentials")){
      return false;
    }
    else{
      const auto& jsonCredentials = jsonConfigRoot.at("credentials");
      if(!jsonCredentials.contains("username")){
        Logger::error("Missing 'username' in config");
        return false;
      }

      if(!jsonCredentials.contains("clientkey")){
        Logger::error("Missing 'clientkey' in config");
        return false;
      }

      m_credentials.emplace(jsonCredentials.at("username"), jsonCredentials.at("clientkey"));
    }

    if(!ready){
      return false;
    }

    if(jsonConfigRoot.contains("profileName")){
      m_profileName.emplace(jsonConfigRoot.at("profileName"));
    }

    if(jsonConfigRoot.contains("subsampleWidth")){
      m_subsampleWidth = jsonConfigRoot.at("subsampleWidth");
    }

    if(jsonConfigRoot.contains("refreshRate")){
      m_refreshRate = jsonConfigRoot.at("refreshRate");
    }

    if(requireSave){
      _save();
    }

    return !jsonConfigRoot.empty();
  }


  void Config::_save() const
  {
    // Parameter that can safelty take a default value
    nlohmann::json jsonOutConfig = {
      {"subsampleWidth", m_subsampleWidth},
      {"refreshRate", m_refreshRate},
      {"restServerPort", m_restServerPort},
      {"boundBackendIP", m_boundBackendIP}
    };

    // Parameters that require user inputs
    if(m_bridgeAddress.has_value()){
      jsonOutConfig["bridgeAddress"] = m_bridgeAddress.value();
    }

    if(m_credentials.has_value()){
      jsonOutConfig["credentials"] = JsonSerializer::serialize(m_credentials.value());
    }

    if(m_profileName.has_value()){
      jsonOutConfig["profileName"] = m_profileName.value();
    }

    if(!std::filesystem::exists(m_configFilePath)){
      std::filesystem::create_directories(m_configFilePath.parent_path());
    }

    std::ofstream configFile(m_configFilePath);
    configFile << jsonOutConfig.dump(2) << "\n";
  }
}
