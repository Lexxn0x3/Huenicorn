#include <Huenicorn/ApiTools.hpp>

#include <nlohmann/json.hpp>

#include <Huenicorn/RequestUtils.hpp>


namespace Huenicorn
{
  namespace ApiTools
  {
    EntertainmentConfigurations loadEntertainmentConfigurations(const std::string& username, const std::string& bridgeAddress)
    {
      EntertainmentConfigurations entertainmentConfigurations;

      RequestUtils::Headers headers = {{"hue-application-key", username}};
      std::string entertainmentConfUrl = "https://" + bridgeAddress + "/clip/v2/resource/entertainment_configuration";
      auto entertainmentConfigurationResponse = RequestUtils::sendRequest(entertainmentConfUrl, "GET", "", headers);

      if(entertainmentConfigurationResponse.at("errors").size() == 0){
        // Listing entertainment configurations
        for(const nlohmann::json& jsonEntertainmentConfiguration : entertainmentConfigurationResponse.at("data")){
          std::string confId = jsonEntertainmentConfiguration.at("id");
          std::string confName = jsonEntertainmentConfiguration.at("metadata").at("name");

          const nlohmann::json& lightServices = jsonEntertainmentConfiguration.at("light_services");

          std::unordered_map<std::string, Device> lights;
          
          for(const nlohmann::json& lightService : lightServices){
            const std::string& lightId = lightService.at("rid");

            std::string lightUrl = "https://" + bridgeAddress + "/clip/v2/resource/light/" + lightId;
            auto jsonLightData = RequestUtils::sendRequest(lightUrl, "GET", "", headers);
            const nlohmann::json& metadata = jsonLightData.at("data").at(0).at("metadata");

            lights.insert({lightId, {lightId, metadata.at("name"), metadata.at("archetype")}});
          }

          Channels channels;
          for(const auto& jsonChannel : jsonEntertainmentConfiguration.at("channels")){
            channels.insert({jsonChannel.at("channel_id").get<uint8_t>(), {false, {}, 0.f}});
          }

          entertainmentConfigurations.insert({confId, {confName, lights, channels}});
        }
      }

      return entertainmentConfigurations;
    }


    Devices loadDevices(const std::string& username, const std::string& bridgeAddress)
    {
      RequestUtils::Headers headers = {{"hue-application-key", username}};
      std::string resourceUrl = "https://" + bridgeAddress + "/clip/v2/resource";
      auto jsonResource = RequestUtils::sendRequest(resourceUrl, "GET", "", headers);

      Devices devices;

      for(const auto& jsonData : jsonResource.at("data")){
        if(jsonData.at("type") == "device"){
          const auto& jsonServices = jsonData.at("services");

          for(const auto& service : jsonServices){
            if(service.at("rtype") == "entertainment"){
              std::string deviceId = service.at("rid");
              std::string name = jsonData.at("metadata").at("name");
              std::string archetype = jsonData.at("metadata").at("archetype");

              devices.emplace(deviceId, Device{deviceId, name, archetype});
            }
          }
        }
      }
      
      return devices;
    }


    EntertainmentConfigurationsChannels loadEntertainmentConfigurationsChannels(const std::string& username, const std::string& bridgeAddress)
    {
      RequestUtils::Headers headers = {{"hue-application-key", username}};
      std::string resourceUrl = "https://" + bridgeAddress + "/clip/v2/resource/entertainment_configuration";

      auto jsonEntertainmentConfigurations = RequestUtils::sendRequest(resourceUrl, "GET", "", headers);

      EntertainmentConfigurationsChannels entertainmentConfigurationsChannels;

      for(const auto& entConf : jsonEntertainmentConfigurations.at("data")){
        std::string configurationId = entConf.at("id");
        for(const auto& jsonChannel : entConf.at("channels")){
          uint8_t channelId = jsonChannel.at("channel_id");
          for(const auto& jsonMember : jsonChannel.at("members")){
            std::string jsonMemberId = jsonMember.at("service").at("rid");
            entertainmentConfigurationsChannels[configurationId][channelId].push_back(jsonMemberId);
          }
        }
      }

      return entertainmentConfigurationsChannels;
    }


    std::vector<Device> matchDevices(const MembersIds& membersIds, const Devices& devices)
    {
      std::vector<Device> matchedDevices;
      for(const auto& memberId : membersIds){
        const auto& it = devices.find(memberId);
        matchedDevices.push_back(it->second);
      }

      return matchedDevices;
    }


    void setStreamingState(const EntertainmentConfigurationEntry& entertainmentConfigurationEntry, const std::string& username, const std::string& bridgeAddress, bool active)
    {
      nlohmann::json jsonBody = {
        {"action", active ? "start" : "stop"},
        {"metadata", {{"name", entertainmentConfigurationEntry.second.name()}}}
      };

      RequestUtils::Headers headers = {{"hue-application-key", username}};

      std::string url = "https://" + bridgeAddress + "/clip/v2/resource/entertainment_configuration/" + entertainmentConfigurationEntry.first;

      RequestUtils::sendRequest(url, "PUT", jsonBody.dump(), headers);
    }


    bool streamingActive(const EntertainmentConfigurationEntry& entertainmentConfigurationEntry, const std::string& username, const std::string& bridgeAddress)
    {
      std::string status;

      RequestUtils::Headers headers = {{"hue-application-key", username}};
      std::string url = "https://" + bridgeAddress + "/clip/v2/resource/entertainment_configuration/" + entertainmentConfigurationEntry.first;
      auto response = RequestUtils::sendRequest(url, "GET", "", headers);
      if(response.at("errors").size() == 0){
        status = response.at("data").front().at("status");
      }

      return status == "active";
    }
  }
}
