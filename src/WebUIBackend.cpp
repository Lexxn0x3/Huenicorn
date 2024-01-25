#include <Huenicorn/WebUIBackend.hpp>

#include <fstream>
#include <sstream>

#include <nlohmann/json.hpp>

#include <Huenicorn/HuenicornCore.hpp>
#include <Huenicorn/JsonSerializer.hpp>
#include <Huenicorn/Logger.hpp>


namespace Huenicorn
{
  WebUIBackend::WebUIBackend(HuenicornCore* huenicornCore):
  IRestServer("webroot"),
  m_huenicornCore(huenicornCore)
  {
    {
      auto resource = std::make_shared<restbed::Resource>();
      resource->set_path("/webUIStatus");
      resource->set_method_handler("GET", [this](SharedSession session){_getWebUIStatus(session);});
      m_service.publish(resource);
    }

    {
      auto resource = std::make_shared<restbed::Resource>();
      resource->set_path("/entertainmentConfigurations");
      resource->set_method_handler("GET", [this](SharedSession session){_getEntertainmentConfigurations(session);});
      m_service.publish(resource);
    }

    {
      auto resource = std::make_shared<restbed::Resource>();
      resource->set_path("/channel/{channelId: .+}");
      resource->set_method_handler("GET", [this](SharedSession session){_getChannel(session);});
      m_service.publish(resource);
    }

    {
      auto resource = std::make_shared<restbed::Resource>();
      resource->set_path("/channels");
      resource->set_method_handler("GET", [this](SharedSession session){_getChannels(session);});
      m_service.publish(resource);
    }

    {
      auto resource = std::make_shared<restbed::Resource>();
      resource->set_path("/displayInfo");
      resource->set_method_handler("GET", [this](SharedSession session){_getDisplayInfo(session);});
      m_service.publish(resource);
    }

    {
      auto resource = std::make_shared<restbed::Resource>();
      resource->set_path("/setEntertainmentConfiguration");
      resource->set_method_handler("PUT", [this](SharedSession session){_setEntertainmentConfiguration(session);});
      m_service.publish(resource);
    }

    {
      auto resource = std::make_shared<restbed::Resource>();
      resource->set_path("/setChannelUV/{channelId: .+}");
      resource->set_method_handler("PUT", [this](SharedSession session){_setChannelUV(session);});
      m_service.publish(resource);
    }

    {
      auto resource = std::make_shared<restbed::Resource>();
      resource->set_path("/setChannelGammaFactor/{channelId: .+}");
      resource->set_method_handler("PUT", [this](SharedSession session){_setChannelGammaFactor(session);});
      m_service.publish(resource);
    }

    {
      auto resource = std::make_shared<restbed::Resource>();
      resource->set_path("/setSubsampleWidth");
      resource->set_method_handler("PUT", [this](SharedSession session){_setSubsampleWidth(session);});
      m_service.publish(resource);
    }

    {
      auto resource = std::make_shared<restbed::Resource>();
      resource->set_path("/setRefreshRate");
      resource->set_method_handler("PUT", [this](SharedSession session){_setRefreshRate(session);});
      m_service.publish(resource);
    }

    {
      auto resource = std::make_shared<restbed::Resource>();
      resource->set_path("/setChannelActivity");
      resource->set_method_handler("POST", [this](SharedSession session){_setChannelActivity(session);});
      m_service.publish(resource);
    }

    {
      auto resource = std::make_shared<restbed::Resource>();
      resource->set_path("/saveProfile");
      resource->set_method_handler("POST", [this](SharedSession session){_saveProfile(session);});
      m_service.publish(resource);
    }

    {
      auto resource = std::make_shared<restbed::Resource>();
      resource->set_path("/stop");
      resource->set_method_handler("POST", [this](SharedSession session){_stop(session);});
      m_service.publish(resource);
    }

    m_webfileBlackList.insert("setup.html");
  }


  void WebUIBackend::_onStart()
  {
    std::stringstream ss;
    ss << "Started web service on port " << m_settings->get_port();
    Logger::log(ss.str());

    if(m_readyWebUIPromise.has_value()){
      m_readyWebUIPromise.value().set_value(true);
    }
  }


  void WebUIBackend::_getVersion(const SharedSession& session) const
  {
    nlohmann::json jsonResponse = {
      {"version", m_huenicornCore->version()},
    };

    std::string response = jsonResponse.dump();

    session->close(restbed::OK, response, {
      {"Content-Length", std::to_string(response.size())},
      {"Content-Type", "application/json"}
    });
  }


  void WebUIBackend::_getWebUIStatus(const SharedSession& session) const
  {
    nlohmann::json jsonResponse = {
      {"ready", true},
    };

    std::string response = jsonResponse.dump();

    session->close(restbed::OK, response, {
      {"Content-Length", std::to_string(response.size())},
      {"Content-Type", "application/json"}
    });
  }


  void WebUIBackend::_getEntertainmentConfigurations(const SharedSession& session) const
  {
    const auto request = session->get_request();

    auto entertainmentConfigurations = JsonSerializer::serialize(m_huenicornCore->entertainmentConfigurations());
    std::string currentEntertainmentConfigurationId = m_huenicornCore->currentEntertainmentConfigurationId().value();

    nlohmann::json jsonResponse = {
      {"entertainmentConfigurations", entertainmentConfigurations},
      {"currentEntertainmentConfigurationId", currentEntertainmentConfigurationId}
    };

    std::string response = jsonResponse.dump();

    session->close(restbed::OK, response, {
      {"Content-Length", std::to_string(response.size())},
      {"Content-Type", "application/json"}
    });
  }


  void WebUIBackend::_getChannel(const SharedSession& session) const
  {
    const auto request = session->get_request();

    uint8_t channelId = stoi(request->get_path_parameter("channelId"));
    std::string response = JsonSerializer::serialize(m_huenicornCore->channels().at(channelId)).dump();
    session->close(restbed::OK, response, {
      {"Content-Length", std::to_string(response.size())},
      {"Content-Type", "application/json"}
    });
  }


  void WebUIBackend::_getChannels(const SharedSession& session) const
  {
    std::string response = JsonSerializer::serialize(m_huenicornCore->channels()).dump();
    session->close(restbed::OK, response, {
      {"Content-Length", std::to_string(response.size())},
      {"Content-Type", "application/json"}
    });
  }


  void WebUIBackend::_getDisplayInfo(const SharedSession& session) const
  {
    auto displayResolution = m_huenicornCore->displayResolution();
    auto subsampleResolutionCandidates = m_huenicornCore->subsampleResolutionCandidates();

    nlohmann::json jsonSubsampleCandidates = nlohmann::json::array();
    for(const auto& candidate : this->m_huenicornCore->subsampleResolutionCandidates()){
      jsonSubsampleCandidates.push_back({
        {"x", candidate.x},
        {"y", candidate.y}
      });
    }

    nlohmann::json jsonDisplayInfo{
      {"x", displayResolution.x},
      {"y", displayResolution.y},
      {"subsampleWidth", m_huenicornCore->subsampleWidth()},
      {"subsampleResolutionCandidates", jsonSubsampleCandidates},
      {"selectedRefreshRate", m_huenicornCore->refreshRate()},
      {"maxRefreshRate", m_huenicornCore->maxRefreshRate()}
    };

    std::string response = jsonDisplayInfo.dump();

    session->close(restbed::OK, response, {
      {"Content-Length", std::to_string(response.size())},
      {"Content-Type", "application/json"}
    });
  }


  void WebUIBackend::_setEntertainmentConfiguration(const SharedSession& session) const
  {
    const auto request = session->get_request();
    int contentLength = request->get_header("Content-Length", 0);

    session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
      std::string data(reinterpret_cast<const char*>(body.data()), body.size());

      std::string entertainmentConfigurationId = nlohmann::json::parse(data);

      bool succeeded = m_huenicornCore->setEntertainmentConfiguration(entertainmentConfigurationId);

      nlohmann::json jsonResponse = {
        {"succeeded", succeeded},
        {"entertainmentConfigurationId", entertainmentConfigurationId},
        {"channels", JsonSerializer::serialize(m_huenicornCore->channels())}
      };

      std::string response = jsonResponse.dump();

      session->close(restbed::OK, response, {
        {"Content-Length", std::to_string(response.size())},
        {"Content-Type", "application/json"}
      });
    });
  }


  void WebUIBackend::_setChannelUV(const SharedSession& session) const
  {
    const auto request = session->get_request();
    int contentLength = request->get_header("Content-Length", 0);

    session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
      std::string data(reinterpret_cast<const char*>(body.data()), body.size());
      const auto& request = session->get_request();
      uint8_t channelId = stoi(request->get_path_parameter("channelId"));

      nlohmann::json jsonUV = nlohmann::json::parse(data);

      float x = jsonUV.at("x");
      float y = jsonUV.at("y");
      UVCorner uvCorner = static_cast<UVCorner>(jsonUV.at("type").get<int>());

      const auto& clampedUVs = m_huenicornCore->setChannelUV(channelId, {x, y}, uvCorner);

      // TODO : Serialize from JsonSerializer
      nlohmann::json jsonResponse = {
        {"uvA", {{"x", clampedUVs.min.x}, {"y", clampedUVs.min.y}}},
        {"uvB", {{"x", clampedUVs.max.x}, {"y", clampedUVs.max.y}}}
      };

      std::string response = jsonResponse.dump();
      
      session->close(restbed::OK, response, {
        {"Content-Length", std::to_string(response.size())},
        {"Content-Type", "application/json"}
      });
    });
  }


  void WebUIBackend::_setChannelGammaFactor(const SharedSession& session) const
  {
    const auto& request = session->get_request();
    int contentLength = request->get_header("Content-Length", 0);

    session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
      std::string data(reinterpret_cast<const char*>(body.data()), body.size());
      nlohmann::json jsonGammaFactorData = nlohmann::json::parse(data);
      const auto& request = session->get_request();

      uint8_t channelId = stoi(request->get_path_parameter("channelId"));

      float gammaFactor = jsonGammaFactorData.at("gammaFactor");

      if(!m_huenicornCore->setChannelGammaFactor(channelId, gammaFactor)){
        std::string response = nlohmann::json{
          {"succeeded", false},
          {"error", "invalid channel id"}
        }.dump();
        session->close(restbed::OK, response, {{"Content-Length", std::to_string(response.size())}});
        return;
      }

      nlohmann::json jsonResponse = nlohmann::json{
        {"succeeded", true},
        {"gammaFactor", gammaFactor}
      };

      std::string response = jsonResponse.dump();
      session->close(restbed::OK, response, {
        {"Content-Length", std::to_string(response.size())},
        {"Content-Type", "application/json"}
      });
    });
  }


  void WebUIBackend::_setSubsampleWidth(const SharedSession& session) const
  {
    const auto request = session->get_request();
    int contentLength = request->get_header("Content-Length", 0);

    session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
      std::string data(reinterpret_cast<const char*>(body.data()), body.size());

      int subsampleWidth = nlohmann::json::parse(data).get<int>();

      m_huenicornCore->setSubsampleWidth(subsampleWidth);

      glm::ivec2 displayResolution = m_huenicornCore->displayResolution();
      nlohmann::json jsonDisplay{
        {"x", displayResolution.x},
        {"y", displayResolution.y},
        {"subsampleWidth", m_huenicornCore->subsampleWidth()}
      };

      std::string response = jsonDisplay.dump();

      session->close(restbed::OK, response, {
        {"Content-Length", std::to_string(response.size())},
        {"Content-Type", "application/json"}
      });
    });
  }


  void WebUIBackend::_setRefreshRate(const SharedSession& session) const
  {
    const auto request = session->get_request();
    int contentLength = request->get_header("Content-Length", 0);

    session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
      std::string data(reinterpret_cast<const char*>(body.data()), body.size());

      unsigned refreshRate = nlohmann::json::parse(data).get<unsigned>();

      m_huenicornCore->setRefreshRate(refreshRate);

      nlohmann::json jsonRefreshRate{
        {"refreshRate", m_huenicornCore->refreshRate()}
      };

      std::string response = jsonRefreshRate.dump();

      session->close(restbed::OK, response, {
        {"Content-Length", std::to_string(response.size())},
        {"Content-Type", "application/json"}
      });
    });
  }


  void WebUIBackend::_setChannelActivity(const SharedSession& session) const
  {
    const auto request = session->get_request();
    int contentLength = request->get_header("Content-Length", 0);

    session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
      std::string data(reinterpret_cast<const char*>(body.data()), body.size());
      nlohmann::json jsonChannelData = nlohmann::json::parse(data);

      uint8_t channelId = jsonChannelData.at("channelId");
      bool active = jsonChannelData.at("active");

      if(!m_huenicornCore->setChannelActivity(channelId, active)){
        std::string response = nlohmann::json{
          {"succeeded", false},
          {"error", "invalid channel id"}
        }.dump();
        session->close(restbed::OK, response, {{"Content-Length", std::to_string(response.size())}});
        return;
      }

      nlohmann::json jsonResponse = nlohmann::json{
        {"succeeded", true},
        {"channels", JsonSerializer::serialize(m_huenicornCore->channels())},
      };
      
      if(active){
        jsonResponse["newActiveChannelId"] = channelId;
      }

      std::string response = jsonResponse.dump();

      session->close(restbed::OK, response, {
        {"Content-Length", std::to_string(response.size())},
        {"Content-Type", "application/json"}
      });
    });
  }


  void WebUIBackend::_saveProfile(const SharedSession& session) const
  {
    m_huenicornCore->saveProfile();

    nlohmann::json jsonResponse = {
      "succeeded", true
    };

    std::string response = jsonResponse.dump();
    session->close(restbed::OK, response, {
      {"Content-Length", std::to_string(response.size())},
      {"Content-Type", "application/json"}
    });
  }


  void WebUIBackend::_stop(const SharedSession& session) const
  {
    nlohmann::json jsonResponse = {{
      "succeeded", true
    }};

    std::string response = jsonResponse.dump();
    session->close(restbed::OK, response, {
      {"Content-Length", std::to_string(response.size())},
      {"Content-Type", "application/json"}
    });

    m_huenicornCore->stop();
  }
}
