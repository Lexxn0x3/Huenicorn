#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include <Huenicorn/Device.hpp>
#include <Huenicorn/EntertainmentConfig.hpp>

namespace Huenicorn
{

  using MembersIds = std::vector<std::string>;
  using ChannelsMembersIds = std::unordered_map<uint8_t, MembersIds>;
  using ChannelsMembers = std::unordered_map<uint8_t, std::vector<Device>>;
  using ConfigurationsChannels = std::unordered_map<std::string, ChannelsMembersIds>;

  namespace ApiTools
  {
    EntertainmentConfigs loadEntertainmentConfigurations(const std::string& username, const std::string& address);
    Devices loadDevices(const std::string& username, const std::string& address);
    ConfigurationsChannels loadConfigurationsChannels(const std::string& username, const std::string& address);
    std::vector<Device> matchDevices(const MembersIds& membersIds, const Devices& devices);
    void setSelectedConfigStreamActivity(bool active, const EntertainmentConfigEntry& entertainmentConfigurationEntry, const std::string& username, const std::string& address);
    bool streamingActive(const EntertainmentConfigEntry& entertainmentConfigurationEntry, const std::string& username, const std::string& address);
  }
}