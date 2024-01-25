#pragma once

#include <filesystem>
#include <optional>

#include <nlohmann/json.hpp>

#include <Huenicorn/Credentials.hpp>


namespace Huenicorn
{
  /**
   * @brief Main configuration parameters to be stored and loaded in a persistent way
   * 
   */
  class Config
  {
  public:
    // Constructor
    /**
     * @brief Config constructor
     * 
     * @param configRoot Path to the configuration directory
     */
    Config(const std::filesystem::path& configRoot);


    // Getters
    /**
     * @brief Returns path to config file
     * 
     * @return const std::filesystem::path& config file path
     */
    const std::filesystem::path& configFilePath() const;


    /**
     * @brief Returns whether the required fields were registered
     * 
     * @return true Required fileds were all provided
     * @return false Some required fields were not provided
     */
    bool initialSetupOk() const;


    /**
     * @brief Returns the registered REST port for web UI
     * 
     * @return int Registered REST port
     */
    int restServerPort() const;


    /**
     * @brief Returns the registered bound ip address for the backend
     * 
     * @return const std::string& Bound ip address
     */
    const std::string& boundBackendIP() const;


    /**
     * @brief Returns the registered address of the Hue bridge
     * 
     * @return const std::optional<std::string>& Registered address of the Hue bridge
     */
    const std::optional<std::string>& bridgeAddress() const;


    /**
     * @brief Returns the registered username from credentials
     * 
     * @return const std::string& Registered username
     */
    const std::string& username() const;


    /**
     * @brief Returns the registered clientkey from credentials
     * 
     * @return const std::string& Registered clientkey
     */
    const std::string& clientkey() const;


    /**
     * @brief Returns registered credentials
     * 
     * @return const std::optional<Credentials>& Registered credentials
     */
    const std::optional<Credentials>& credentials() const;


    /**
     * @brief Returns the registered profile name
     * 
     * @return const std::optional<std::string>& Registered profile name
     */
    const std::optional<std::string>& profileName() const;


    /**
     * @brief Returns the Registered refresh rate
     * 
     * @return unsigned Registered refresh rate
     */
    unsigned refreshRate() const;


    /**
     * @brief Returns the registered subsample with
     * 
     * @return unsigned Registered subsample width
     */
    unsigned subsampleWidth() const;

    // Setters
    /**
     * @brief Registers Hue bridge address
     * 
     * @param bridgeAddress Address of the Hue bridge
     */
    void setBridgeAddress(const std::string& bridgeAddress);


    /**
     * @brief Registers credentials
     * 
     * @param username Username for the Hue bridge
     * @param clientkey Client key for the Hue bridge
     */
    void setCredentials(const std::string& username, const std::string& clientkey);


    /**
     * @brief Registers current profile name
     * 
     * @param profileName Profile name to use
     */
    void setProfileName(const std::string& profileName);


    /**
     * @brief registers the subsample width for image processing
     * 
     * @param subsampleWidth Subsample width
     */
    void setSubsampleWidth(unsigned subsampleWidth);


    /**
     * @brief Registers the refresh rate for color streaming
     * 
     * @param refreshRate Refresh rate
     */
    void setRefreshRate(unsigned refreshRate);


  private:
    // Private methods
    /**
     * @brief Loads the configuration data from config file
     * 
     * @return true Required fields were loaded
     * @return false Required fields are missing
     */
    bool _loadConfigFile();


    /**
     * @brief Writes the current state of the config. Called from all setters
     * 
     */
    void _save() const;


    // Attributes
    std::filesystem::path m_configFilePath;
    int m_restServerPort{8080};
    std::string m_boundBackendIP{"0.0.0.0"};
    std::optional<std::string> m_bridgeAddress;
    std::optional<Credentials> m_credentials;
    std::optional<std::string> m_profileName;
    unsigned m_refreshRate{0};
    unsigned m_subsampleWidth{0};
  };
}
