#include <Huenicorn/ApiTools.hpp>

#include <iostream>

#include <nlohmann/json.hpp>

#include <glm/gtx/string_cast.hpp>

#include <Huenicorn/RequestUtils.hpp>



using namespace nlohmann;
using namespace std;

namespace Huenicorn
{
  namespace ApiTools
  {
    inline GamutCoordinates jsonToGamut(const nlohmann::json& jsonGamut)
    {
      glm::vec2 r{jsonGamut.at("red").at("x"), jsonGamut.at("red").at("y")};
      glm::vec2 g{jsonGamut.at("green").at("x"), jsonGamut.at("green").at("y")};
      glm::vec2 b{jsonGamut.at("blue").at("x"), jsonGamut.at("blue").at("y")};
      GamutCoordinates gcs{r, g, b};

      return gcs;
    }


    EntertainmentConfigurations loadEntertainmentConfigurations(const string& username, const string& bridgeAddress)
    {
      EntertainmentConfigurations entertainmentConfigurations;

      RequestUtils::Headers headers = {{"hue-application-key", username}};
      string entertainmentConfUrl = "https://" + bridgeAddress + "/clip/v2/resource/entertainment_configuration";
      auto entertainmentConfigurationResponse = RequestUtils::sendRequest(entertainmentConfUrl, "GET", "", headers);

      if(entertainmentConfigurationResponse.at("errors").size() == 0){
        // Listing entertainment configurations
        for(const json& jsonEntertainmentConfiguration : entertainmentConfigurationResponse.at("data")){
          string confId = jsonEntertainmentConfiguration.at("id");
          string confName = jsonEntertainmentConfiguration.at("metadata").at("name");

          const json& lightServices = jsonEntertainmentConfiguration.at("light_services");

          unordered_map<string, Device> lights;
          
          for(const json& lightService : lightServices){
            const string& lightId = lightService.at("rid");

            string lightUrl = "https://" + bridgeAddress + "/clip/v2/resource/light/" + lightId;
            auto jsonLightData = RequestUtils::sendRequest(lightUrl, "GET", "", headers);
            const json& metadata = jsonLightData.at("data").at(0).at("metadata");

            GamutCoordinates gamutCoordinates = jsonToGamut(jsonLightData.at("data").at(0).at("color").at("gamut"));
            lights.insert({lightId, {Device{lightId, metadata.at("name"), metadata.at("archetype"), gamutCoordinates}}});

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


    Devices loadDevices(const string& username, const string& bridgeAddress)
    {
      RequestUtils::Headers headers = {{"hue-application-key", username}};
      string resourceUrl = "https://" + bridgeAddress + "/clip/v2/resource";
      auto jsonResource = RequestUtils::sendRequest(resourceUrl, "GET", "", headers);

      Devices devices;

      std::unordered_map<string, GamutCoordinates> devicesGamuts;
      for(const auto& jsonData : jsonResource.at("data")){
        if(jsonData.at("type") == "light"){
          devicesGamuts.insert({jsonData.at("id"), jsonToGamut(jsonData.at("color").at("gamut"))});
          cout << jsonData.at("id") << " " << jsonData.at("color").at("gamut") << endl;
        }
      }


      for(const auto& jsonData : jsonResource.at("data")){
        if(jsonData.at("type") == "device"){
          const auto& jsonServices = jsonData.at("services");

          for(const auto& service : jsonServices){
            if(service.at("rtype") == "entertainment"){
              string deviceId = service.at("rid");
              string name = jsonData.at("metadata").at("name");
              string archetype = jsonData.at("metadata").at("archetype");

              devices.emplace(deviceId, Device{deviceId, name, archetype, {}});
            }
          }
        }
      }
      
      return devices;
    }


    EntertainmentConfigurationsChannels loadEntertainmentConfigurationsChannels(const string& username, const string& bridgeAddress)
    {
      RequestUtils::Headers headers = {{"hue-application-key", username}};
      string resourceUrl = "https://" + bridgeAddress + "/clip/v2/resource/entertainment_configuration";

      auto jsonEntertainmentConfigurations = RequestUtils::sendRequest(resourceUrl, "GET", "", headers);

      EntertainmentConfigurationsChannels entertainmentConfigurationsChannels;

      for(const auto& entConf : jsonEntertainmentConfigurations.at("data")){
        string configurationId = entConf.at("id");
        for(const auto& jsonChannel : entConf.at("channels")){
          uint8_t channelId = jsonChannel.at("channel_id");
          for(const auto& jsonMember : jsonChannel.at("members")){
            string jsonMemberId = jsonMember.at("service").at("rid");
            entertainmentConfigurationsChannels[configurationId][channelId].push_back(jsonMemberId);
          }
        }
      }

      return entertainmentConfigurationsChannels;
    }


    vector<std::pair<std::string, Device>> matchDevices(const MembersIds& membersIds, const Devices& devices)
    {
      vector<std::pair<std::string, Device>> matchedDevices;
      for(const auto& memberId : membersIds){
        const auto& it = devices.find(memberId);
        matchedDevices.push_back(make_pair(it->first, it->second));
      }

      return matchedDevices;
    }


    void setStreamingState(const EntertainmentConfigurationEntry& entertainmentConfigurationEntry, const string& username, const string& bridgeAddress, bool active)
    {
      json jsonBody = {
        {"action", active ? "start" : "stop"},
        {"metadata", {{"name", entertainmentConfigurationEntry.second.name()}}}
      };

      RequestUtils::Headers headers = {{"hue-application-key", username}};

      string url = "https://" + bridgeAddress + "/clip/v2/resource/entertainment_configuration/" + entertainmentConfigurationEntry.first;

      RequestUtils::sendRequest(url, "PUT", jsonBody.dump(), headers);
    }


    bool streamingActive(const EntertainmentConfigurationEntry& entertainmentConfigurationEntry, const std::string& username, const std::string& bridgeAddress)
    {
      string status;

      RequestUtils::Headers headers = {{"hue-application-key", username}};
      string url = "https://" + bridgeAddress + "/clip/v2/resource/entertainment_configuration/" + entertainmentConfigurationEntry.first;
      auto response = RequestUtils::sendRequest(url, "GET", "", headers);
      if(response.at("errors").size() == 0){
        status = response.at("data").front().at("status");
      }

      return status == "active";
    }
  }
}
