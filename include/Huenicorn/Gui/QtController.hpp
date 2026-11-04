#pragma once

#include <QObject>
#include <QString>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>

namespace Huenicorn
{
  class HuenicornCore;
  class ChannelListModel;
  class EntertainmentConfigurationModel;

  /**
   * @brief Bridge between the Qt Quick GUI and HuenicornCore.
   *
   * Exposes the core state as QML properties and the core actions as invokable
   * methods. Instanciated once in main() and exposed to QML as the "huenicorn"
   * context property.
   */
  class QtController : public QObject
  {
    Q_OBJECT

    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(QString configFilePath READ configFilePath CONSTANT)
    Q_PROPERTY(bool setupRequired READ setupRequired NOTIFY setupRequiredChanged)
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(int selectedChannelId READ selectedChannelId WRITE setSelectedChannelId NOTIFY selectedChannelIdChanged)
    Q_PROPERTY(ChannelListModel* channels READ channelsModel CONSTANT)
    Q_PROPERTY(EntertainmentConfigurationModel* entertainmentConfigurations READ entertainmentConfigurationsModel CONSTANT)
    Q_PROPERTY(QVariantList subsampleCandidates READ subsampleCandidates NOTIFY displayInfoChanged)
    Q_PROPERTY(int subsampleWidth READ subsampleWidth NOTIFY displayInfoChanged)
    Q_PROPERTY(int refreshRate READ refreshRate NOTIFY displayInfoChanged)
    Q_PROPERTY(int maxRefreshRate READ maxRefreshRate NOTIFY displayInfoChanged)
    Q_PROPERTY(int displayWidth READ displayWidth NOTIFY displayInfoChanged)
    Q_PROPERTY(int displayHeight READ displayHeight NOTIFY displayInfoChanged)
    Q_PROPERTY(QVariantList interpolations READ interpolations NOTIFY interpolationInfoChanged)
    Q_PROPERTY(int interpolation READ interpolation NOTIFY interpolationInfoChanged)
    Q_PROPERTY(QString currentEntertainmentConfigurationId READ currentEntertainmentConfigurationId NOTIFY entertainmentConfigurationsChanged)
    Q_PROPERTY(int channelsVersion READ channelsVersion NOTIFY channelsChanged)

  public:
    /**
     * @brief QtController constructor
     * 
     * @param core Pointer to Huenicorn core
     * @param parent Parent QObject
     */
    explicit QtController(HuenicornCore* core, QObject* parent = nullptr);


    // Getters
    QString version() const;
    QString configFilePath() const;
    bool setupRequired() const;
    bool running() const;
    int selectedChannelId() const;
    ChannelListModel* channelsModel() const;
    EntertainmentConfigurationModel* entertainmentConfigurationsModel() const;
    QVariantList subsampleCandidates() const;
    int subsampleWidth() const;
    int refreshRate() const;
    int maxRefreshRate() const;
    int displayWidth() const;
    int displayHeight() const;
    QVariantList interpolations() const;
    int interpolation() const;
    QString currentEntertainmentConfigurationId() const;
    int channelsVersion() const;


    // Setter
    void setSelectedChannelId(int channelId);


    // Setup invokables
    /**
     * @brief Queries the Hue discovery service for bridges on the local network
     * 
     * @return QVariantMap {succeeded, bridges: [{id, internalipaddress}], error?}
     */
    Q_INVOKABLE QVariantMap autodetectBridges();

    /**
     * @brief Validates and registers a Hue bridge address
     * 
     * @param bridgeAddress Bridge address (IP or URL)
     * @return true Bridge address is valid and was registered
     */
    Q_INVOKABLE bool validateBridgeAddress(const QString& bridgeAddress);

    /**
     * @brief Registers a new user on the configured Hue bridge (link button must be pressed)
     * 
     * @return QVariantMap {succeeded, username?, clientkey?, error?}
     */
    Q_INVOKABLE QVariantMap registerNewUser();


    // Runtime invokables
    Q_INVOKABLE bool selectEntertainmentConfiguration(const QString& entertainmentConfigurationId);
    Q_INVOKABLE bool setChannelActivity(int channelId, bool active);
    Q_INVOKABLE QVariantMap setChannelUV(int channelId, int corner, double x, double y);
    Q_INVOKABLE bool setChannelGammaFactor(int channelId, double gammaFactor);
    Q_INVOKABLE void setSubsampleWidth(int subsampleWidth);
    Q_INVOKABLE void setRefreshRate(int refreshRate);
    Q_INVOKABLE void setInterpolation(int interpolation);
    Q_INVOKABLE void saveProfile();
    Q_INVOKABLE void quit();

    /**
     * @brief Returns the data of a channel as a map (empty if unknown or not running)
     * 
     * @param channelId Channel ID
     * @return QVariantMap Channel data
     */
    Q_INVOKABLE QVariantMap channelData(int channelId) const;


  signals:
    void setupRequiredChanged();
    void runningChanged();
    void selectedChannelIdChanged();
    void displayInfoChanged();
    void interpolationInfoChanged();
    void entertainmentConfigurationsChanged();
    void channelsChanged();


  private slots:
    /**
     * @brief Polls the core state and emits change signals when needed
     * 
     */
    void _poll();


  private:
    // Private methods
    void _refreshDisplayInfo();
    void _refreshInterpolations();
    void _reloadChannels();
    void _reloadEntertainmentConfigurations();


    // Attributes
    HuenicornCore* m_core;
    ChannelListModel* m_channelsModel;
    EntertainmentConfigurationModel* m_entertainmentConfigurationsModel;
    QTimer m_pollTimer;

    int m_selectedChannelId{-1};
    bool m_wasSetupRequired{true};
    bool m_wasRunning{false};
    QString m_currentEntertainmentConfigurationId;
    int m_channelsVersion{0};

    QVariantList m_subsampleCandidates;
    int m_subsampleWidth{0};
    int m_refreshRate{0};
    int m_maxRefreshRate{0};
    int m_displayWidth{0};
    int m_displayHeight{0};
    QVariantList m_interpolations;
    int m_interpolation{0};
  };
}
