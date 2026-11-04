// Huenicorn headers (pulling in Crow) must come before Qt headers because of
// the Qt "signals" macro clashing with Crow's signals() method
#include <Huenicorn/HuenicornCore.hpp>
#include <Huenicorn/Interpolation.hpp>
#include <Huenicorn/UV.hpp>

#include <Huenicorn/Gui/QtController.hpp>
#include <Huenicorn/Gui/ChannelListModel.hpp>
#include <Huenicorn/Gui/EntertainmentConfigurationModel.hpp>

#include <QCoreApplication>
#include <QStringList>


namespace Huenicorn
{
  QtController::QtController(HuenicornCore* core, QObject* parent):
  QObject(parent),
  m_core(core),
  m_channelsModel(new ChannelListModel(core, this)),
  m_entertainmentConfigurationsModel(new EntertainmentConfigurationModel(core, this))
  {
    m_pollTimer.setInterval(500);
    connect(&m_pollTimer, &QTimer::timeout, this, &QtController::_poll);
    m_pollTimer.start();
  }


  QString QtController::version() const
  {
    return QString::fromStdString(m_core->version());
  }


  QString QtController::configFilePath() const
  {
    return QString::fromStdString(m_core->configFilePath().string());
  }


  bool QtController::setupRequired() const
  {
    return !m_core->initialSetupOk();
  }


  bool QtController::running() const
  {
    return m_core->running();
  }


  int QtController::selectedChannelId() const
  {
    return m_selectedChannelId;
  }


  void QtController::setSelectedChannelId(int channelId)
  {
    if(m_selectedChannelId == channelId){
      return;
    }

    m_selectedChannelId = channelId;
    emit selectedChannelIdChanged();
  }


  ChannelListModel* QtController::channelsModel() const
  {
    return m_channelsModel;
  }


  EntertainmentConfigurationModel* QtController::entertainmentConfigurationsModel() const
  {
    return m_entertainmentConfigurationsModel;
  }


  QVariantList QtController::subsampleCandidates() const
  {
    return m_subsampleCandidates;
  }


  int QtController::subsampleWidth() const
  {
    return m_subsampleWidth;
  }


  int QtController::refreshRate() const
  {
    return m_refreshRate;
  }


  int QtController::maxRefreshRate() const
  {
    return m_maxRefreshRate;
  }


  int QtController::displayWidth() const
  {
    return m_displayWidth;
  }


  int QtController::displayHeight() const
  {
    return m_displayHeight;
  }


  QVariantList QtController::interpolations() const
  {
    return m_interpolations;
  }


  int QtController::interpolation() const
  {
    return m_interpolation;
  }


  QString QtController::currentEntertainmentConfigurationId() const
  {
    return m_currentEntertainmentConfigurationId;
  }


  int QtController::channelsVersion() const
  {
    return m_channelsVersion;
  }


  QVariantMap QtController::autodetectBridges()
  {
    nlohmann::json result = m_core->autodetectedBridge();

    QVariantMap map;
    map["succeeded"] = result.value("succeeded", false);

    if(result.contains("error")){
      map["error"] = QString::fromStdString(result.at("error").get<std::string>());
    }

    if(result.contains("bridges")){
      QVariantList bridges;
      for(const auto& bridge : result.at("bridges")){
        QVariantMap bridgeMap;
        bridgeMap["id"] = QString::fromStdString(bridge.value("id", ""));
        bridgeMap["internalipaddress"] = QString::fromStdString(bridge.value("internalipaddress", ""));
        bridges.append(bridgeMap);
      }
      map["bridges"] = bridges;
    }

    return map;
  }


  bool QtController::validateBridgeAddress(const QString& bridgeAddress)
  {
    QString sanitized = bridgeAddress.trimmed();
    while(sanitized.endsWith('/')){
      sanitized.chop(1);
    }

    if(sanitized.isEmpty()){
      return false;
    }

    return m_core->validateBridgeAddress(sanitized.toStdString());
  }


  QVariantMap QtController::registerNewUser()
  {
    nlohmann::json result = m_core->registerNewUser();

    QVariantMap map;
    map["succeeded"] = result.value("succeeded", false);

    if(result.contains("error")){
      map["error"] = QString::fromStdString(result.at("error").get<std::string>());
    }
    if(result.contains("username")){
      map["username"] = QString::fromStdString(result.at("username").get<std::string>());
    }
    if(result.contains("clientkey")){
      map["clientkey"] = QString::fromStdString(result.at("clientkey").get<std::string>());
    }

    if(map["succeeded"].toBool()){
      // The core start() loop polls the config, so the streaming will begin on its own
      emit setupRequiredChanged();
    }

    return map;
  }


  bool QtController::selectEntertainmentConfiguration(const QString& entertainmentConfigurationId)
  {
    bool succeeded = m_core->setEntertainmentConfiguration(entertainmentConfigurationId.toStdString());

    if(succeeded){
      _reloadChannels();
      _reloadEntertainmentConfigurations();
      setSelectedChannelId(-1);
    }

    return succeeded;
  }


  bool QtController::setChannelActivity(int channelId, bool active)
  {
    bool succeeded = m_core->setChannelActivity(static_cast<uint8_t>(channelId), active);

    if(succeeded){
      _reloadChannels();
    }

    return succeeded;
  }


  QVariantMap QtController::setChannelUV(int channelId, int corner, double x, double y)
  {
    QVariantMap map;

    UVCorner uvCorner = static_cast<UVCorner>(corner);
    const UVs& clamped = m_core->setChannelUV(static_cast<uint8_t>(channelId), UV{static_cast<float>(x), static_cast<float>(y)}, uvCorner);

    map["uvMinX"] = clamped.min.x;
    map["uvMinY"] = clamped.min.y;
    map["uvMaxX"] = clamped.max.x;
    map["uvMaxY"] = clamped.max.y;

    _reloadChannels();

    return map;
  }


  bool QtController::setChannelGammaFactor(int channelId, double gammaFactor)
  {
    bool succeeded = m_core->setChannelGammaFactor(static_cast<uint8_t>(channelId), static_cast<float>(gammaFactor));

    if(succeeded){
      _reloadChannels();
    }

    return succeeded;
  }


  void QtController::setSubsampleWidth(int subsampleWidth)
  {
    m_core->setSubsampleWidth(static_cast<unsigned>(subsampleWidth));
    _refreshDisplayInfo();
  }


  void QtController::setRefreshRate(int refreshRate)
  {
    if(!m_core->running()){
      return;
    }

    m_core->setRefreshRate(static_cast<unsigned>(refreshRate));
    _refreshDisplayInfo();
  }


  void QtController::setInterpolation(int interpolation)
  {
    m_core->setInterpolation(static_cast<unsigned>(interpolation));
    _refreshInterpolations();
  }


  void QtController::saveProfile()
  {
    m_core->saveProfile();
  }


  void QtController::quit()
  {
    m_core->stop();
    QCoreApplication::quit();
  }


  QVariantMap QtController::channelData(int channelId) const
  {
    QVariantMap map;

    if(!m_core->running()){
      return map;
    }

    const Channels& channels = m_core->channels();
    auto it = channels.find(static_cast<uint8_t>(channelId));
    if(it == channels.end()){
      return map;
    }

    const Channel& channel = it->second;
    const UVs& uvs = channel.uvs();

    QStringList devices;
    for(const Device& device : channel.devices()){
      devices << QString::fromStdString(device.name);
    }

    map["channelId"] = static_cast<int>(it->first);
    map["active"] = channel.state() == Channel::State::Active;
    map["gammaFactor"] = channel.gammaFactor();
    map["uvMinX"] = uvs.min.x;
    map["uvMinY"] = uvs.min.y;
    map["uvMaxX"] = uvs.max.x;
    map["uvMaxY"] = uvs.max.y;
    map["devices"] = devices;
    map["label"] = QString("Channel %1").arg(it->first + 1);

    return map;
  }


  void QtController::_poll()
  {
    bool setupReq = setupRequired();
    bool isRunning = running();

    if(setupReq != m_wasSetupRequired){
      m_wasSetupRequired = setupReq;
      emit setupRequiredChanged();
    }

    if(isRunning != m_wasRunning){
      m_wasRunning = isRunning;
      emit runningChanged();

      if(isRunning){
        _reloadEntertainmentConfigurations();
        _reloadChannels();
        _refreshDisplayInfo();
        _refreshInterpolations();
      }
    }
  }


  void QtController::_refreshDisplayInfo()
  {
    if(!m_core->running()){
      return;
    }

    glm::ivec2 resolution = m_core->displayResolution();
    m_displayWidth = resolution.x;
    m_displayHeight = resolution.y;

    m_subsampleCandidates.clear();
    for(const auto& candidate : m_core->subsampleResolutionCandidates()){
      QVariantMap candidateMap;
      candidateMap["x"] = candidate.x;
      candidateMap["y"] = candidate.y;
      m_subsampleCandidates.append(candidateMap);
    }

    m_subsampleWidth = static_cast<int>(m_core->subsampleWidth());
    m_refreshRate = static_cast<int>(m_core->refreshRate());
    m_maxRefreshRate = static_cast<int>(m_core->maxRefreshRate());

    emit displayInfoChanged();
  }


  void QtController::_reloadChannels()
  {
    m_channelsModel->reload();
    ++m_channelsVersion;
    emit channelsChanged();
  }


  void QtController::_reloadEntertainmentConfigurations()
  {
    m_entertainmentConfigurationsModel->reload();

    auto currentId = m_core->currentEntertainmentConfigurationId();
    m_currentEntertainmentConfigurationId = currentId.has_value() ? QString::fromStdString(currentId.value()) : QString();

    emit entertainmentConfigurationsChanged();
  }


  void QtController::_refreshInterpolations()
  {
    m_interpolations.clear();

    for(const auto& [name, type] : m_core->availableInterpolations()){
      QVariantMap interpolationMap;
      interpolationMap["name"] = QString::fromStdString(name);
      interpolationMap["value"] = static_cast<int>(type);
      m_interpolations.append(interpolationMap);
    }

    m_interpolation = static_cast<int>(m_core->interpolation());

    emit interpolationInfoChanged();
  }
}
