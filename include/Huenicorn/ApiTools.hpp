#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include <Huenicorn/Device.hpp>
#include <Huenicorn/EntertainmentConfiguration.hpp>


namespace Huenicorn
{
  using MembersIds = std::vector<std::string>;
  using ChannelsMembersIds = std::unordered_map<uint8_t, MembersIds>;
  using ChannelsMembers = std::unordered_map<uint8_t, std::vector<Device>>;
  using EntertainmentConfigurationsChannels = std::unordered_map<std::string, ChannelsMembersIds>;


  /**
   * @brief Provides wrappers around the Hue bridge API
   * 
   */
  namespace ApiTools
  {
    /**
     * @brief Loads entertainment configurations from Hue bridge
     * 
     * @param username Username credential for the HTTPS request
     * @param bridgeAddress Address of the Hue bridge
     * @return EntertainmentConfigurations List of entertainment configurations
     */
    EntertainmentConfigurations loadEntertainmentConfigurations(const std::string& username, const std::string& bridgeAddress);

    /**
     * @brief Loads devices from all entertainment configurations
     * 
     * @param username Username credential for the HTTPS request
     * @param bridgeAddress Address of the Hue bridge
     * @return Devices List of entertainment devices
     */
    Devices loadDevices(const std::string& username, const std::string& bridgeAddress);

    /**
     * @brief Loads entertainment configurations channels
     * 
     * @param username Username credential for the HTTPS request
     * @param bridgeAddress Address of the Hue bridge
     * @return EntertainmentConfigurationsChannels Map of entertainment configurations channels
     */
    EntertainmentConfigurationsChannels loadEntertainmentConfigurationsChannels(const std::string& username, const std::string& bridgeAddress);

    /**
     * @brief Resolves members data from list of IDs
     * 
     * @param membersIds Members ids to match
     * @param devices Devices data
     * @return std::vector<Device> List of matched data for each device
     */
    std::vector<std::pair<std::string, Device>> matchDevices(const MembersIds& membersIds, const Devices& devices);

    /**
     * @brief Set the streaming state of the entertainment configuration on the Hue bridge
     * 
     * @param entertainmentConfigurationEntry Entertainment configuration to manage
     * @param username Username credential for the HTTPS request
     * @param bridgeAddress Address of the Hue bridge
     * @param active True for active, false for inactive
     */
    void setStreamingState(const EntertainmentConfigurationEntry& entertainmentConfigurationEntry, const std::string& username, const std::string& bridgeAddress, bool active);

    /**
     * @brief Returns the streaming state of the entertainment configuration entry
     * 
     * @param entertainmentConfigurationEntry 
     * @param username Username credential for the HTTPS request
     * @param bridgeAddress Address of the Hue bridge
     * @return true Streaming is active
     * @return false Streaming is inactive
     */
    bool streamingActive(const EntertainmentConfigurationEntry& entertainmentConfigurationEntry, const std::string& username, const std::string& bridgeAddress);
  }
}
