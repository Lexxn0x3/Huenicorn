#include <Huenicorn/JsonSerializer.hpp>


namespace Huenicorn
{
  namespace JsonSerializer
  {
    nlohmann::json serialize(const Credentials& credentials)
    {
      return {
        {"username", credentials.username()},
        {"clientkey", credentials.clientkey()},
      };
    }


    nlohmann::json serialize(const EntertainmentConfiguration& entertainmentConfiguration)
    {
      return {
        {"name", entertainmentConfiguration.name()},
        //{"devices", JsonSerializer::serialize(entertainmentConfiguration.devices())},
        //{"channels", JsonSerializer::serialize(entertainmentConfiguration.channels())},
      };
    }


    nlohmann::json serialize(const EntertainmentConfigurations& entertainmentConfigurations)
    {
      nlohmann::json jsonEntertainmentConfigurations = nlohmann::json::array();
      for(const auto& entertainmentConfiguration : entertainmentConfigurations){
        auto& it = jsonEntertainmentConfigurations.emplace_back(serialize(entertainmentConfiguration.second));
        it["entertainmentConfigurationId"] = entertainmentConfiguration.first;
      }

      return jsonEntertainmentConfigurations;
    }


    nlohmann::json serialize(const UVs& uvs)
    {
      return {
        {"uvA", {{"x", uvs.min.x}, {"y", uvs.min.y}}},
        {"uvB", {{"x", uvs.max.x}, {"y", uvs.max.y}}}
      };
    }


    nlohmann::json serialize(const Device& device)
    {
      return {
        {"id", device.id},
        {"name", device.name},
        {"type", device.type}
      };
    }


    nlohmann::json serialize(const Devices& devices)
    {
      nlohmann::json jsonDevices = nlohmann::json::array();
      for(const auto& device : devices){
        jsonDevices.push_back(serialize(device.second));
      }

      return jsonDevices;
    }


    nlohmann::json serialize(const Channel& channel)
    {
      return {
        {"active", channel.state() == Channel::State::Active},
        {"uvs", serialize(channel.uvs())},
        {"gammaFactor", channel.gammaFactor()},
        {"devices", serialize(channel.devices())}
      };
    }


    nlohmann::json serialize(const Channels& channels)
    {
      nlohmann::json jsonChannels = nlohmann::json::array();
      for(const auto& channel : channels){
        auto& it = jsonChannels.emplace_back(serialize(channel.second));
        it["channelId"] = channel.first;
      }

      return jsonChannels;
    }


    nlohmann::json serialize(const std::vector<Device>& devices)
    {
      nlohmann::json jsonDevices = nlohmann::json::array();
      for(const auto& device : devices){
        jsonDevices.push_back(serialize(device));
      }

      return jsonDevices;
    }


    nlohmann::json serialize(const ChannelsMembers& channelsDevices)
    {
      nlohmann::json jsonChannelsMembers = nlohmann::json::array();
      for(const auto& channel : channelsDevices){
        jsonChannelsMembers.push_back({
            {"id", channel.first},
            {"members", serialize(channel.second)}
          }
        );
      }

      return jsonChannelsMembers;
    }
  }
}
