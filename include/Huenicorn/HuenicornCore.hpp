#pragma once

#include <filesystem>
#include <optional>
#include <thread>

#include <nlohmann/json.hpp>

#include <Huenicorn/ApiTools.hpp>
#include <Huenicorn/Channel.hpp>
#include <Huenicorn/Config.hpp>
#include <Huenicorn/EntertainmentConfiguration.hpp>
#include <Huenicorn/EntertainmentConfigurationSelector.hpp>
#include <Huenicorn/IGrabber.hpp>
#include <Huenicorn/IRestServer.hpp>
#include <Huenicorn/Streamer.hpp>
#include <Huenicorn/TickSynchronizer.hpp>
#include <Huenicorn/UV.hpp>


namespace Huenicorn
{
  /**
   * @brief Main execution scope of Huenicorn that handles application components and executes streaming loop.
   * 
   */
  class HuenicornCore
  {
    // Type definitions
    struct ThreadedRestService
    {
      std::unique_ptr<IRestServer> server;
      std::optional<std::thread> thread;
    };

  public:
    // Constructor
    /**
     * @brief HuenicornCore constructor
     * 
     * @param version Current version of Huenicorn
     * @param configRoot Path to the configuration directory
     */
    HuenicornCore(const std::string& version, const std::filesystem::path& configRoot);


    // Getters
    /**
     * @brief Returns the current version of Huenicorn
     * 
     * @return const std::string& Current version of Huenicorn
     */
    const std::string& version() const;


    /**
     * @brief Returns the path of the configuration file
     * 
     * @return const std::filesystem::path Configuration file path
     */
    const std::filesystem::path configFilePath() const;


    /**
     * @brief Returns the list of the available channels
     * 
     * @return const Channels&  Channels list
     */
    const Channels& channels() const;


    /**
     * @brief Returns the list of available entertainment configurations
     * 
     * @return const EntertainmentConfigurations& Available entertainment configurations
     */
    const EntertainmentConfigurations& entertainmentConfigurations() const;


    /**
     * @brief Returns the current entertainment configuration
     * 
     * @return const EntertainmentConfiguration& Current entertainment configuration
     */
    const EntertainmentConfiguration& currentEntertainmentConfiguration() const;


    /**
     * @brief Returns the ID of the current entertainment configuration
     * 
     * @return const std::string& Current entertainment configuration ID
     */
    std::optional<std::string> currentEntertainmentConfigurationId() const;


    /**
     * @brief Returns the resolution of the display
     * 
     * @return glm::ivec2 Display resolution
     */
    glm::ivec2 displayResolution() const;


    /**
     * @brief Returns a list of subsample resolutions
     * 
     * @return std::vector<glm::ivec2> Subsample resolutions
     */
    std::vector<glm::ivec2> subsampleResolutionCandidates() const;


    /**
     * @brief Returns the current subsample width
     * 
     * @return unsigned Current subsample width
     */
    unsigned subsampleWidth() const;


    /**
     * @brief Returns the current color streaming refresh rate
     * 
     * @return unsigned Refresh rate
     */
    unsigned refreshRate() const;


    /**
     * @brief Returns the maximal refresh rate defined by the display
     * 
     * @return unsigned Maximal refresh rate
     */
    unsigned maxRefreshRate() const;


    /**
     * @brief Returns the resolved Hue bridge IP address
     * 
     * @return nlohmann::json Object containing Hue bridge address and request status
     */
    nlohmann::json autoDetectedBridge() const;


    /**
     * @brief Requests the addition of a new user on the Hue bridge
     * 
     * @return nlohmann::json Newly created user's credentials
     */
    nlohmann::json registerNewUser();


    // Setters
    /**
     * @brief Sets the current entertainment configuration
     * 
     * @param entertainmentConfigurationId ID of the entertainment configuration to select
     * @return true Successfully selected entertainment configuration
     * @return false Failed to select entertainment configuration
     */
    bool setEntertainmentConfiguration(const std::string& entertainmentConfigurationId);


    /**
     * @brief Set the Channel UV
     * 
     * @param channelId ID of the channel to affect
     * @param uv UV coordinate to set
     * @param uvCorner Corner to affect
     * @return const UVs& Clamped UV value
     */
    const UVs& setChannelUV(uint8_t channelId, UV&& uv, UVCorner uvCorner);


    /**
     * @brief Sets the Channel Gamma Factor
     * 
     * @param channelId ID of the channel to affect
     * @param gammaFactor Gamma factor to apply
     * @return true Gamma factor was successuflly set
     * @return false Gamma factor could not be set
     */
    bool setChannelGammaFactor(uint8_t channelId, float gammaFactor);


    /**
     * @brief Set the width of the subsample image to compute
     * 
     * @param subsampleWidth Desired width for the image subsample
     */
    void setSubsampleWidth(unsigned subsampleWidth);


    /**
     * @brief Set the refresh rate for color streaming
     * 
     * @param refreshRate Desired refresh rate for color data streaming
     */
    void setRefreshRate(unsigned refreshRate);


    // Methods
    /**
     * @brief Initiates Huenicorn and starts the main loop
     * 
     */
    void start();


    /**
     * @brief Stops the main loop
     * 
     */
    void stop();


    /**
     * @brief Registers Hue bridge address if valid
     * 
     * @param bridgeAddress Hue bridge address to register
     * @return true Hue bridge address is valid and was registered
     * @return false Hue bridge address is invalid and was discarded
     */
    bool validateBridgeAddress(const std::string& bridgeAddress);


    /**
     * @brief Registers Hue bridge credentials if valid
     * 
     * @param credentials Hue bridge credentials to register
     * @return true Hue bridge credentials are valid and were registered
     * @return false Hue bridge credentials are invalid and were discarded
     */
    bool validateCredentials(const Credentials& credentials);


    /**
     * @brief Set the streaming activity of the channel referenced by ID
     * 
     * @param channelId ID of the channel to affect
     * @param active State of the streaming
     * @return true Streaming state was successfully set
     * @return false Streaming state could not be set
     */
    bool setChannelActivity(uint8_t channelId, bool active);


    /**
     * @brief Saves the current state of a user-defined configuration as profile
     * 
     */
    void saveProfile();


  private:
    // Private getter
    /**
     * @brief Returns the path to the current profile
     * 
     * @return std::filesystem::path Path to the current profile
     */
    std::filesystem::path _profilePath() const;


    /**
     * @brief Loads the last profile
     * 
     * @return Loaded profile. Empty if not found
     */
    std::optional<nlohmann::json> _getProfile();


    // Private methods

    /**
     * @brief Initializes config and profile
     * 
     * @return true Succeeded to load a suitable configuration
     * @return false Failed to load a suitable configuration
     */
    bool _initSettings();


    /**
     * @brief Starts the initial setup wizard
     *
     * @return true Setup finished success
     * @return false Setup was not completed
    */
    bool _runInitialSetup();


    /**
     * Brief Initializes a grabber based on the graphical session
     * 
     * @return true Relevant grabber was found and initialized
     * @return false could not initialize a grabber
    */
    bool _initGrabber();


    /**
     * Brief Initializes the web UI
    */
    void _initWebUI();


    /**
     * @brief Initializes the channels based on the profile settings
     * 
     * @param jsonProfile Data from the user-defined profile
     */
    void _initChannels(const nlohmann::json& jsonProfile);


    /**
     * @brief Opens the user's default browser on the Huenicorn page
     * 
     */
    void _spawnBrowser();


    /**
     * @brief Enables the selected entertainment configuration, recreates streamer  and loads related profile
     * 
     * @param entertainmentConfigurationId ID of the entertainment configuration to enable
     */
    void _enableEntertainmentConfiguration(const std::string& entertainmentConfigurationId);


    /**
     * @brief Starts the main loop calling for processing at given refresh rate
     * 
     */
    void _startStreamingLoop();


    /**
     * @brief Computes the channels colors and streams them
     * 
     */
    void _processFrame();


    /**
     * @brief Disables streaming on current entertainment configuration
     * 
     */
    void _shutdown();


    // Attributes
    const std::string& m_version;
    std::filesystem::path m_configRoot;
    Config m_config;

    bool m_keepLooping;
    bool m_openedSetup{false};
    std::unique_ptr<TickSynchronizer> m_tickSynchronizer;

    //  API structure wrapper
    std::unique_ptr<EntertainmentConfigurationSelector> m_selector;
    Channels m_channels;

    // Streamer
    std::mutex m_streamerMutex;
    std::unique_ptr<Streamer> m_streamer;

    // Service and flags
    ThreadedRestService m_webUIService;

    //  Image Processing
    std::unique_ptr<IGrabber> m_grabber;
    cv::Mat m_cvImage;
  };
}
