#include <Huenicorn/HuenicornCore.hpp>

#include <iostream>
#include <fstream>

#include <Huenicorn/X11Grabber.hpp>
#include <Huenicorn/ImageProcessing.hpp>
#include <Huenicorn/RequestUtils.hpp>
#include <Huenicorn/SetupBackend.hpp>
#include <Huenicorn/WebUIBackend.hpp>
#include <Huenicorn/JsonSerializer.hpp>


using namespace nlohmann;
using namespace glm;
using namespace std;


namespace Huenicorn
{
  HuenicornCore::HuenicornCore(const std::string& version, const std::filesystem::path& configRoot):
  m_version(version),
  m_configRoot(configRoot),
  m_config(m_configRoot),
  m_grabber(make_unique<X11Grabber>(&m_config))
  {}


  const std::string& HuenicornCore::version() const
  {
    return m_version;
  }


  const std::filesystem::path HuenicornCore::configFilePath() const
  {
    return m_config.configFilePath();
  }


  const Channels& HuenicornCore::channels() const
  {
    return m_channels;
  }


  const EntertainmentConfigurations& HuenicornCore::entertainmentConfigurations() const
  {
    return m_selector->entertainmentConfigurations();
  }


  const EntertainmentConfiguration& HuenicornCore::currentEntertainmentConfiguration() const
  {
    return m_selector->currentEntertainmentConfiguration();
  }


  std::optional<std::string> HuenicornCore::currentEntertainmentConfigurationId() const
  {
    return m_selector->currentEntertainmentConfigurationId();
  }


  glm::ivec2 HuenicornCore::displayResolution() const
  {
    return m_grabber->displayResolution();
  }


  vector<glm::ivec2> HuenicornCore::subsampleResolutionCandidates() const
  {
    return m_grabber->subsampleResolutionCandidates();
  }


  unsigned HuenicornCore::subsampleWidth() const
  {
    return m_config.subsampleWidth();
  }


  unsigned HuenicornCore::refreshRate() const
  {
    return m_config.refreshRate();
  }


  unsigned HuenicornCore::maxRefreshRate() const
  {
    return m_grabber->displayRefreshRate();
  }


  json HuenicornCore::autoDetectedBridge() const
  {
    string bridgeAddress;
    try{
      auto detectedBridgeData = RequestUtils::sendRequest("https://discovery.meethue.com/", "GET");

      if(detectedBridgeData.size() < 1){
        return {{"succeeded", false}, {"error", "Could not autodetect bridge."}};
      }

      bridgeAddress = detectedBridgeData.front().at("internalipaddress");
    }
    catch(const json::exception& e){
      return {{"succeeded", false}, {"error", "Could not autodetect bridge."}};
    }

    return {{"succeeded", bridgeAddress != ""}, {"bridgeAddress", bridgeAddress}};
  }


  json HuenicornCore::registerNewUser()
  {
    string login = getlogin();
    string deviceType = "huenicorn#" + login;

    json request = {{"devicetype", deviceType}, {"generateclientkey", true}};
    auto response = RequestUtils::sendRequest(m_config.bridgeAddress().value() + "/api", "POST", request.dump());

    if(response.size() < 1){
      return {{"succeeded", false}, {"error", "unreachable bridge"}};
    }

    if(response.at(0).contains("error")){
      return {{"succeeded", false}, {"error", response.at(0).at("error").at("description")}};
    }

    string username = response.at(0).at("success").at("username");
    string clientkey = response.at(0).at("success").at("clientkey");
    m_config.setCredentials(username, clientkey);

    return {{"succeeded", true}, {"username", username}, {"clientkey", clientkey}};
  }


  bool HuenicornCore::setEntertainmentConfiguration(const std::string& entertainmentConfigurationId)
  {
    if(!m_selector->selectEntertainmentConfiguration(entertainmentConfigurationId)){
      return false;
    }

    _enableEntertainmentConfiguration(entertainmentConfigurationId);

    return true;
  }


  const UVs& HuenicornCore::setChannelUV(uint8_t channelId, UV&& uv, UVCorner uvCorner)
  {
    return m_channels.at(channelId).setUV(std::move(uv), uvCorner);
  }


  bool HuenicornCore::setChannelGammaFactor(uint8_t channelId, float gammaExponent)
  {
    if(m_channels.find(channelId) == m_channels.end()){
      return false;
    }

    m_channels.at(channelId).setGammaFactor(gammaExponent);
    return true;
  }


  void HuenicornCore::setSubsampleWidth(unsigned subsampleWidth)
  {
    m_config.setSubsampleWidth(subsampleWidth);
  }


  void HuenicornCore::setRefreshRate(unsigned refreshRate)
  {
    refreshRate = std::min(refreshRate, m_grabber->displayRefreshRate());

    m_config.setRefreshRate(refreshRate);
    refreshRate = m_config.refreshRate();
    m_tickSynchronizer->setTickInterval(1.0f / refreshRate);
  }


  void HuenicornCore::start()
  {
    if(!_initSettings()){
      cout << "Could not load suitable entertainment configuration." << endl;
      return;

      // TODO : Add tool to create entertainment configurations inside Huenicorn
      // so the official application would no longer be a requirement
    }

    _startStreamingLoop();
  }


  void HuenicornCore::stop()
  {
    m_keepLooping = false;
  }


  bool HuenicornCore::validateBridgeAddress(const std::string& bridgeAddress)
  {
    try{
      string sanitizedAddress = bridgeAddress;
      while (sanitizedAddress.back() == '/'){
        sanitizedAddress.pop_back();
      }

      auto response = RequestUtils::sendRequest(sanitizedAddress + "/api", "GET", "");
      if(response.size() == 0){
        return false;
      }

      m_config.setBridgeAddress(sanitizedAddress);
    }
    catch(const json::exception& exception){
      cout << exception.what() << endl;
      return false;
    }

    return true;
  }


  bool HuenicornCore::validateCredentials(const Credentials& credentials)
  {
    try{
      auto response = RequestUtils::sendRequest(m_config.bridgeAddress().value() + "/api/" + credentials.username(), "GET", "");
      if(response.size() == 0){
        return false;
      }

      if(response.is_array() && response.at(0).contains("error")){
        return false;
      }

      m_config.setCredentials(credentials.username(), credentials.clientkey());

      cout << "Successfully registered credentials" << endl;
    }
    catch(const json::exception& exception){
      cout << exception.what() << endl;
      return false;
    }

    return true;
  }


  bool HuenicornCore::setChannelActivity(uint8_t channelId, bool active)
  {
    if(m_channels.find(channelId) == m_channels.end()){
      return false;
    }

    m_channels.at(channelId).setActive(active);
    return true;
  }


  void HuenicornCore::saveProfile()
  {
    if(!m_selector->validSelection()){
      cout << "There is currently no valid entertainment configuration selected." << endl;
      return;
    }

    nlohmann::json profile;
    if(m_selector->validSelection()){
      profile = json{
        {"entertainmentConfigurationId", m_selector->currentEntertainmentConfigurationId().value()},
        {"channels", JsonSerializer::serialize(m_channels)}
      };
    }

    if(!m_config.profileName().has_value()){
      m_config.setProfileName("profile");
    }

    ofstream profileFile(_profilePath(), ofstream::out);
    profileFile << profile.dump(2) << endl;
    profileFile.close();
  }


  filesystem::path HuenicornCore::_profilePath() const
  {
    if(!m_config.profileName().has_value()){
      return {};
    }

    return m_configRoot / std::filesystem::path(m_config.profileName().value()).replace_extension("json");
  }


  optional<json> HuenicornCore::_getProfile()
  {
    filesystem::path profilePath = _profilePath();
    json jsonProfile = json::object();

    if(!profilePath.empty() && filesystem::exists(profilePath) && filesystem::is_regular_file(profilePath)){
      ifstream profileFile(profilePath);
      return json::parse(profileFile);
    }

    return nullopt;
  }


  bool HuenicornCore::_initSettings()
  {
    unsigned port = m_config.restServerPort();
    const std::string& boundBackendIP = m_config.boundBackendIP();
    bool openedSetup = false;

    if(!m_config.initialSetupOk()){
      cout << "Starting setup backend" << endl;

      SetupBackend sb(this);
      sb.start(port, boundBackendIP);

      if(sb.aborted()){
        cout << "Initial setup was aborted" << endl;
        return false;
      }

      cout << "Finished setup" << endl;
      openedSetup = true;
    }

    if(!m_config.initialSetupOk()){
      cout << "There are errors in the config file" << endl;
      return false;
    }

    if(m_config.refreshRate() == 0){
      m_config.setRefreshRate(m_grabber->displayRefreshRate());
    }

    if(m_config.subsampleWidth() == 0){
      m_config.setSubsampleWidth(m_grabber->subsampleResolutionCandidates().back().x);
    }

    cout << "Configuration is ready. Feel free to modify it manually by editing " << std::quoted(m_config.configFilePath().string()) << endl;

    const Credentials& credentials = m_config.credentials().value();
    const string& bridgeAddress =  m_config.bridgeAddress().value();

    m_selector = make_unique<EntertainmentConfigurationSelector>(credentials, bridgeAddress);

    unsigned restServerPort = m_config.restServerPort();
    m_webUIService.server = make_unique<WebUIBackend>(this);
    m_webUIService.thread.emplace([&](){
      m_webUIService.server->start(restServerPort, boundBackendIP);
    });

    string entertainmentConfigurationId = {};
    auto optJsonProfile = _getProfile();

    if(optJsonProfile.has_value()){
      const auto& jsonProfile = optJsonProfile.value();
      if(jsonProfile.contains("entertainmentConfigurationId")){
        entertainmentConfigurationId = jsonProfile.at("entertainmentConfigurationId");
      }
    }

    _enableEntertainmentConfiguration(entertainmentConfigurationId);

    if(!optJsonProfile.has_value() && !openedSetup){
      thread spawnBrowser([this](){_spawnBrowser();});
      spawnBrowser.detach();
    }

    return true;
  }


  void HuenicornCore::_initChannels(const nlohmann::json& jsonChannels)
  {
    const string& username = m_config.credentials().value().username();
    const string& bridgeAddress =  m_config.bridgeAddress().value();
    Devices devices = ApiTools::loadDevices(username, bridgeAddress);
    EntertainmentConfigurationsChannels entertainmentConfigurationsChannels = ApiTools::loadEntertainmentConfigurationsChannels(username, bridgeAddress);

    Channels channels;

    for(const auto& [id, channel] : m_selector->currentEntertainmentConfiguration().channels()){
      bool found = false;
      const auto& members = ApiTools::matchDevices(entertainmentConfigurationsChannels.at(m_selector->currentEntertainmentConfigurationId().value()).at(id), devices);
      for(const auto& jsonProfileChannel : jsonChannels){
        if(jsonProfileChannel.at("channelId") == id){
          bool active = jsonProfileChannel.at("active");
          json jsonUVs = jsonProfileChannel.at("uvs");
          float uvAx = jsonUVs.at("uvA").at("x");
          float uvAy = jsonUVs.at("uvA").at("y");
          float uvBx = jsonUVs.at("uvB").at("x");
          float uvBy = jsonUVs.at("uvB").at("y");

          UVs uvs = {{uvAx, uvAy}, {uvBx, uvBy}};
          float gammaFactor = jsonProfileChannel.at("gammaFactor");
          channels.emplace(id, Channel{active, members, gammaFactor, uvs});

          found = true;
          break;
        }
      }

      if(!found){
        channels.emplace(id, Channel{false, members, 0.0f});
      }
    }

    m_channels = std::move(channels);
  }


  void HuenicornCore::_spawnBrowser()
  {
    while (!m_webUIService.server->running()){
      this_thread::sleep_for(100ms);
    }
    
    stringstream serviceUrlStream;
    serviceUrlStream << "http://127.0.0.1:" << m_config.restServerPort();
    string serviceURL = serviceUrlStream.str();
    std::cout << "Management WebUI is ready and available at " << serviceURL << std::endl;

    if(system(string("xdg-open " + serviceURL).c_str()) != 0){
      std::cout << "Failed to open browser" << std::endl;
    }
  }


  void HuenicornCore::_enableEntertainmentConfiguration(const std::string& entertainmentConfigurationId)
  {
    if(!m_selector->selectEntertainmentConfiguration(entertainmentConfigurationId)){
      return;
    }

    const Credentials& credentials = m_config.credentials().value();

    {
      lock_guard lock(m_streamerMutex);
      m_streamer = make_unique<Streamer>(credentials, m_config.bridgeAddress().value());
      m_streamer->setEntertainmentConfigurationId(m_selector->currentEntertainmentConfigurationId().value());
      m_streamer->setColorSpace(Streamer::ColorSpace::XYZ);
    }

    auto profilePath = _profilePath();
    json jsonChannels = json::object();

    if(filesystem::is_regular_file(profilePath)){
      json jsonProfile = json::parse(ifstream(_profilePath()));
      if(jsonProfile.contains("channels")){
        jsonChannels = jsonProfile.at("channels");
      }
    }

    _initChannels(jsonChannels);
  }


  void HuenicornCore::_startStreamingLoop()
  {
    m_tickSynchronizer = make_unique<TickSynchronizer>(1.0f / static_cast<float>(m_config.refreshRate()));

    m_tickSynchronizer->start();

    m_keepLooping = true;
    while(m_keepLooping){
      _processFrame();

      if(!m_tickSynchronizer->sync()){
        const auto& lastExcess = m_tickSynchronizer->lastExcess();
        float percentage = lastExcess.rate * 100;
        ostringstream warningMessage;
        warningMessage << "Scheduled interval has been exceeded of ";
        warningMessage << lastExcess.extra;
        warningMessage << " (";
        warningMessage << percentage;
        warningMessage << "%).";
        cout << warningMessage.str() << endl;
        cout << "Please reduce refresh rate if this warning persists." << endl;
      }
    }

    _shutdown();

    m_webUIService.server->stop();
    m_webUIService.thread.value().join();
  }


  void HuenicornCore::_processFrame()
  {
    m_grabber->grabFrameSubsample(m_cvImage);
    cv::Mat subframeImage;
    ChannelStreams channelStreams;

    for(auto& [channelId, channel] : m_channels){
      if(channel.state() == Channel::State::Inactive){
        continue;
      }

      Color color;
      if(channel.state() == Channel::State::PendingShutdown){
        channel.acknowledgeShutdown();
      }
      else{
        const auto& uvs = channel.uvs();

        glm::ivec2 a{uvs.min.x * m_cvImage.cols, uvs.min.y * m_cvImage.rows};
        glm::ivec2 b{uvs.max.x * m_cvImage.cols, uvs.max.y * m_cvImage.rows};

        ImageProcessing::getSubImage(m_cvImage, a, b).copyTo(subframeImage);
        color = ImageProcessing::getDominantColors(subframeImage, 1).front();
      }

      vec3 outputColor;
      switch(m_streamer->colorSpace()){
        case Streamer::ColorSpace::RGB:
        {
          glm::vec3 rgbColor = color.asRGB();
          outputColor = glm::pow(rgbColor, glm::vec3(channel.gammaExponent()));
          break;
        }

        case Streamer::ColorSpace::XYZ:
        {
          glm::vec3 xyzColor = color.asXYZ(); // ToDo :  Get gamut coordinates to enable clamp
          outputColor = {xyzColor.x, xyzColor.y, glm::pow(xyzColor.z, channel.gammaExponent())};
          break;
        }
      }

      channelStreams.push_back({channelId, outputColor});
    }

    {
      lock_guard lock(m_streamerMutex);
      if(m_streamer.get()){
        m_streamer->streamChannels(channelStreams);
      }
    }
  }


  void HuenicornCore::_shutdown()
  {
    m_selector->disableStreaming();
  }
}
