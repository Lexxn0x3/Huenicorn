#include <Huenicorn/RestServer.hpp>

#include <iostream>
#include <fstream>

#include <nlohmann/json.hpp>

#include <Huenicorn/HuenicornCore.hpp>

using namespace nlohmann;
using namespace std;

namespace Huenicorn
{
  RestServer::RestServer(HuenicornCore* huenicornCore):
  m_huenicornCore(huenicornCore),
  m_webroot("webroot")
  {
    m_settings = make_shared<restbed::Settings>();
    m_settings->set_default_headers({
      {"Connection", "close"},
      {"Access-Control-Allow-Origin", "*"}
    });

    m_contentTypes = {
      {".js", "text/javascript"},
      {".html", "text/html"},
      {".css", "text/css"}
    };

    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/");
      resource->set_method_handler("GET", [this](SharedSession session){_getWebFile(session);});
      m_service.publish(resource);
    }

    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/{webFileName: [a-zA-Z0-9]+(\\.)[a-zA-Z0-9]+}");
      resource->set_method_handler("GET", [this](SharedSession session){_getWebFile(session);});
      m_service.publish(resource);
    }

    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/availableLights");
      resource->set_method_handler("GET", [this](SharedSession session){_getAvailableLights(session);});
      m_service.publish(resource);
    }

    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/syncedLights");
      resource->set_method_handler("GET", [this](SharedSession session){_getSyncedLights(session);});
      m_service.publish(resource);
    }

    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/syncedLight/{lightId: .+}");
      resource->set_method_handler("GET", [this](SharedSession session){_getSyncedLight(session);});
      m_service.publish(resource);
    }

    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/allLights");
      resource->set_method_handler("GET", [this](SharedSession session){_getAllLights(session);});
      m_service.publish(resource);
    }

    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/displayInfo");
      resource->set_method_handler("GET", [this](SharedSession session){_getDisplayInfo(session);});
      m_service.publish(resource);
    }

    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/transitionTime_c");
      resource->set_method_handler("GET", [this](SharedSession session){_getTransitionTime_c(session);});
      m_service.publish(resource);
    }

    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/setLightUV/{lightId: .+}");
      resource->set_method_handler("PUT", [this](SharedSession session){_setLightUV(session);});
      m_service.publish(resource);
    }

    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/setLightGammaFactor/{lightId: .+}");
      resource->set_method_handler("PUT", [this](SharedSession session){_setLightGammaFactor(session);});
      m_service.publish(resource);
    }

    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/setSubsampleWidth");
      resource->set_method_handler("PUT", [this](SharedSession session){_setSubsampleWidth(session);});
      m_service.publish(resource);
    }

    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/setRefreshRate");
      resource->set_method_handler("PUT", [this](SharedSession session){_setRefreshRate(session);});
      m_service.publish(resource);
    }
    
    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/setTransitionTime_c");
      resource->set_method_handler("PUT", [this](SharedSession session){_setTransitionTime_c(session);});
      m_service.publish(resource);
    }

    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/syncLight");
      resource->set_method_handler("POST", [this](SharedSession session){_syncLight(session);});
      m_service.publish(resource);
    }
    
    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/unsyncLight");
      resource->set_method_handler("POST", [this](SharedSession session){_unsyncLight(session);});
      m_service.publish(resource);
    }

    {
      auto resource = make_shared<restbed::Resource>();
      resource->set_path("/saveProfile");
      resource->set_method_handler("POST", [this](SharedSession session){_saveProfile(session);});
      m_service.publish(resource);
    }
  }


  RestServer::~RestServer()
  {
    _stop();
  }


  bool RestServer::start(int port)
  {
    if(m_serviceThread.has_value()){
      return false;
    }

    m_settings->set_port(port);

    m_serviceThread.emplace([this](){m_service.start(m_settings);});
    cout << "Web UI ready and available at http://127.0.0.1:" << port << endl;

    return true;
  }


  void RestServer::stop()
  {
    _stop();
  }


  bool RestServer::_stop()
  {
    if(!m_serviceThread.has_value()){
      return false;
    }

    m_service.stop();
    m_serviceThread.value().join();

    return true;
  }


  void RestServer::_getAvailableLights(const SharedSession& session) const
  {
    const auto request = session->get_request();
    int contentLength = request->get_header("Content-Length", 0);

    session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
      (void)body;
      string response = m_huenicornCore->jsonAvailableLights().dump();
      session->close(restbed::OK, response, {
        {"Content-Length", std::to_string(response.size())},
        {"Content-Type", "application/json"}
      });
    });
  }


  void RestServer::_getSyncedLights(const SharedSession& session) const
  {
    const auto request = session->get_request();
    int contentLength = request->get_header("Content-Length", 0);

    session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
      (void)body;
      string response = m_huenicornCore->jsonSyncedLights().dump();
      session->close(restbed::OK, response, {
        {"Content-Length", std::to_string(response.size())},
        {"Content-Type", "application/json"}
      });
    });
  }


  void RestServer::_getSyncedLight(const SharedSession& session) const
  {
    const auto request = session->get_request();
    int contentLength = request->get_header("Content-Length", 0);
    string lightId = request->get_path_parameter("lightId");

    session->fetch(contentLength, [this, lightId](const SharedSession& session, const restbed::Bytes& body){
      (void)body;
      SharedSyncedLight syncedLight = m_huenicornCore->syncedLight(lightId);

      json jsonLight = syncedLight ? syncedLight->serialize() : json::object();
      string response = jsonLight.dump();

      session->close(restbed::OK, response, {
        {"Content-Length", std::to_string(response.size())},
        {"Content-Type", "application/json"}
      });
    });
  }


  void RestServer::_getAllLights(const SharedSession& session) const
  {
    const auto request = session->get_request();
    int contentLength = request->get_header("Content-Length", 0);

    session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
      (void)body;
      string response = m_huenicornCore->jsonAllLights().dump();
      session->close(restbed::OK, response, {
        {"Content-Length", std::to_string(response.size())},
        {"Content-Type", "application/json"}
      });
    });
  }


  void RestServer::_getDisplayInfo(const SharedSession& session) const
  {
    const auto request = session->get_request();
    int contentLength = request->get_header("Content-Length", 0);

    session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
      (void)body;
      glm::ivec2 screenResolution = m_huenicornCore->screenResolution();
      auto subsampleResolutionCandidates = m_huenicornCore->subsampleResolutionCandidates();

      json jsonSubsampleCandidates = json::array();
      for(const auto& candidate : this->m_huenicornCore->subsampleResolutionCandidates()){
        jsonSubsampleCandidates.push_back({
          {"x", candidate.x},
          {"y", candidate.y}
        });
      }

      json jsonDisplayInfo{
        {"x", screenResolution.x},
        {"y", screenResolution.y},
        {"subsampleWidth", this->m_huenicornCore->subsampleWidth()},
        {"subsampleResolutionCandidates", jsonSubsampleCandidates}
      };

      string response = jsonDisplayInfo.dump();

      session->close(restbed::OK, response, {
        {"Content-Length", std::to_string(response.size())},
        {"Content-Type", "application/json"}
      });
    });
  }


  void RestServer::_getTransitionTime_c(const SharedSession& session) const
  {
    const auto request = session->get_request();
    int contentLength = request->get_header("Content-Length", 0);

    session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
      (void)body;

      json jsonTransitionTime{
        {"transitionTime", this->m_huenicornCore->transitionTime_c()},
      };

      string response = jsonTransitionTime.dump();

      session->close(restbed::OK, response, {
        {"Content-Length", std::to_string(response.size())},
        {"Content-Type", "application/json"}
      });
    });
  }


  void RestServer::_getWebFile(const SharedSession& session) const
  {
    const auto request = session->get_request();
    filesystem::path webFileName = request->get_path_parameter("webfileName");

    if(webFileName == ""){
      webFileName = "index.html";
    }

    filesystem::path webFileFullPath = m_webroot / webFileName;

    if(!filesystem::exists(webFileFullPath)){
      webFileName = "404.html";
      webFileFullPath = m_webroot / webFileName;
    }

    string extension = webFileName.extension().string();
    string contentType = "text/plain";

    if(m_contentTypes.find(extension) != m_contentTypes.end()){
      contentType = m_contentTypes.at(extension);
    }

    fstream webFile(webFileFullPath);
    string response = string(std::istreambuf_iterator<char>(webFile), std::istreambuf_iterator<char>());

    std::multimap<string, string> headers{
      {"Content-Length", std::to_string(response.size())},
      {"Content-Type", contentType}
    };

    // ToDo MIME headers
    session->close(restbed::OK, response, headers);
  }


  void RestServer::_setLightUV(const SharedSession& session) const
  {
    const auto request = session->get_request();
    int contentLength = request->get_header("Content-Length", 0);

    session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
      string data(reinterpret_cast<const char*>(body.data()), body.size());
      const auto& request = session->get_request();
      string lightId = request->get_path_parameter("lightId");

      json jsonUV = json::parse(data);

      float x = jsonUV.at("x");
      float y = jsonUV.at("y");
      SyncedLight::UVType uvType = static_cast<SyncedLight::UVType>(jsonUV.at("type").get<int>());

      const auto& clampedUVs = m_huenicornCore->setLightUV(lightId, {x, y}, uvType);

      json jsonResponse = {
        {"uvA", {{"x", clampedUVs.min.x}, {"y", clampedUVs.min.y}}},
        {"uvB", {{"x", clampedUVs.max.x}, {"y", clampedUVs.max.y}}}
      };

      string response = jsonResponse.dump();
      
      session->close(restbed::OK, response, {
        {"Content-Length", std::to_string(response.size())},
        {"Content-Type", "application/json"}
      });
    });
  }


  void RestServer::_setLightGammaFactor(const SharedSession& session) const
  {
    const auto& request = session->get_request();
    int contentLength = request->get_header("Content-Length", 0);

    session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
      string data(reinterpret_cast<const char*>(body.data()), body.size());
      json jsonGammaFactorData = json::parse(data);
      const auto& request = session->get_request();
      string lightId = request->get_path_parameter("lightId");

      float gammaFactor = jsonGammaFactorData.at("gammaFactor");
      const auto& availableLights = m_huenicornCore->availableLights();
      if(availableLights.find(lightId) == availableLights.end()){
        string response = json{
          {"status", false},
          {"error", "key not found"}
        }.dump();
        session->close(restbed::OK, response, {{"Content-Length", std::to_string(response.size())}});
        return;
      }

      m_huenicornCore->setLightGammaFactor(lightId, gammaFactor);

      json jsonResponse = json{
        {"status", true},
        {"gammaFactor", gammaFactor}
      };

      string response = jsonResponse.dump();
      session->close(restbed::OK, response, {
        {"Content-Length", std::to_string(response.size())},
        {"Content-Type", "application/json"}
      });
    });
  }


  void RestServer::_setSubsampleWidth(const SharedSession& session) const
  {
    const auto request = session->get_request();
    int contentLength = request->get_header("Content-Length", 0);

    session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
      string data(reinterpret_cast<const char*>(body.data()), body.size());

      int subsampleWidth = json::parse(data).get<int>();

      m_huenicornCore->setSubsampleWidth(subsampleWidth);

      glm::ivec2 screenResolution = m_huenicornCore->screenResolution();
      json jsonScreen{
        {"x", screenResolution.x},
        {"y", screenResolution.y},
        {"subsampleWidth", m_huenicornCore->subsampleWidth()}
      };

      string response = jsonScreen.dump();

      session->close(restbed::OK, response, {
        {"Content-Length", std::to_string(response.size())},
        {"Content-Type", "application/json"}
      });
    });
  }


  void RestServer::_setRefreshRate(const SharedSession& session) const
  {
    const auto request = session->get_request();
    int contentLength = request->get_header("Content-Length", 0);

    session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
      string data(reinterpret_cast<const char*>(body.data()), body.size());

      unsigned refreshRate = json::parse(data).get<unsigned>();

      m_huenicornCore->setRefreshRate(refreshRate);

      json jsonRefreshRate{
        {"refreshRate", m_huenicornCore->refreshRate()}
      };

      string response = jsonRefreshRate.dump();

      session->close(restbed::OK, response, {
        {"Content-Length", std::to_string(response.size())},
        {"Content-Type", "application/json"}
      });
    });
  }


  void RestServer::_setTransitionTime_c(const SharedSession& session) const
  {
    const auto request = session->get_request();
    int contentLength = request->get_header("Content-Length", 0);

    session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
      string data(reinterpret_cast<const char*>(body.data()), body.size());

      unsigned refreshRate = json::parse(data).get<unsigned>();

      m_huenicornCore->setTransitionTime_c(refreshRate);

      json jsonTransitionTime{
        {"transitionTime_c", m_huenicornCore->transitionTime_c()}
      };

      string response = jsonTransitionTime.dump();

      session->close(restbed::OK, response, {
        {"Content-Length", std::to_string(response.size())},
        {"Content-Type", "application/json"}
      });
    });
  }


  void RestServer::_syncLight(const SharedSession& session) const
  {
    const auto request = session->get_request();
    int contentLength = request->get_header("Content-Length", 0);

    session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
      string data(reinterpret_cast<const char*>(body.data()), body.size());
      json jsonLightData = json::parse(data);

      string lightId = jsonLightData.at("id");
      const auto& availableLights = m_huenicornCore->availableLights();
      if(availableLights.find(lightId) == availableLights.end()){
        string response = json{
          {"status", false},
          {"error", "key not found"}
        }.dump();
        session->close(restbed::OK, response, {{"Content-Length", std::to_string(response.size())}});
        return;
      }

      json jsonResponse = json::object();
      bool succeeded = m_huenicornCore->addSyncedLight(lightId) != nullptr;
      if(succeeded){
        jsonResponse["newSyncedLightId"] = lightId;
      }

      jsonResponse["status"] = succeeded;
      jsonResponse["lights"] = m_huenicornCore->jsonAllLights();

      string response = jsonResponse.dump();
      session->close(restbed::OK, response, {
        {"Content-Length", std::to_string(response.size())},
        {"Content-Type", "application/json"}
      });
    });
  }


  void RestServer::_unsyncLight(const SharedSession& session) const
  {
    const auto request = session->get_request();
    int contentLength = request->get_header("Content-Length", 0);

    session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
      string data(reinterpret_cast<const char*>(body.data()), body.size());
      json jsonLightData = json::parse(data);

      string lightId = jsonLightData.at("id");

      json jsonResponse = json::object();
      bool succeeded = m_huenicornCore->removeSyncedLight(lightId);
      if(succeeded){
        jsonResponse["unsyncedLightId"] = lightId;
      }

      jsonResponse["status"] = succeeded;
      jsonResponse["lights"] = m_huenicornCore->jsonAllLights();

      string response = jsonResponse.dump();
      session->close(restbed::OK, response, {
        {"Content-Length", std::to_string(response.size())},
        {"Content-Type", "application/json"}
      });
    });
  }


  void RestServer::_saveProfile(const SharedSession& session) const
  {
    const auto request = session->get_request();
    int contentLength = request->get_header("Content-Length", 0);

    session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
      (void)body;
      m_huenicornCore->saveProfile();

      json jsonResponse = {
        "status", true
      };

      string response = jsonResponse.dump();
      session->close(restbed::OK, response, {
        {"Content-Length", std::to_string(response.size())},
        {"Content-Type", "application/json"}
      });
    });
  }
}
