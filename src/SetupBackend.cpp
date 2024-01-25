#include <Huenicorn/SetupBackend.hpp>

#include <chrono>

#include <Huenicorn/HuenicornCore.hpp>
#include <Huenicorn/Logger.hpp>

using namespace std::chrono_literals;

namespace Huenicorn
{
  SetupBackend::SetupBackend(HuenicornCore* huenicornCore):
  IRestServer("webroot"),
  m_huenicornCore(huenicornCore)
  {
    m_indexFile = "setup.html";

    m_contentTypes = {
      {".js", "text/javascript"},
      {".html", "text/html"},
      {".css", "text/css"}
    };

    {
      auto resource = std::make_shared<restbed::Resource>();
      resource->set_path("/finishSetup");
      resource->set_method_handler("POST", [this](SharedSession session){_finish(session);});
      m_service.publish(resource);
    }

    {
      auto resource = std::make_shared<restbed::Resource>();
      resource->set_path("/abort");
      resource->set_method_handler("POST", [this](SharedSession session){_abort(session);});
      m_service.publish(resource);
    }

    {
      auto resource = std::make_shared<restbed::Resource>();
      resource->set_path("/autoDetectBridge");
      resource->set_method_handler("GET", [this](SharedSession session){_autoDetectBridge(session);});
      m_service.publish(resource);
    }
    
    {
      auto resource = std::make_shared<restbed::Resource>();
      resource->set_path("/configFilePath");
      resource->set_method_handler("GET", [this](SharedSession session){_configFilePath(session);});
      m_service.publish(resource);
    }

    {
      auto resource = std::make_shared<restbed::Resource>();
      resource->set_path("/validateBridgeAddress");
      resource->set_method_handler("PUT", [this](SharedSession session){_validateBridgeAddress(session);});
      m_service.publish(resource);
    }

    {
      auto resource = std::make_shared<restbed::Resource>();
      resource->set_path("/validateCredentials");
      resource->set_method_handler("PUT", [this](SharedSession session){_validateCredentials(session);});
      m_service.publish(resource);
    }

    {
      auto resource = std::make_shared<restbed::Resource>();
      resource->set_path("/registerNewUser");
      resource->set_method_handler("PUT", [this](SharedSession session){_registerNewUser(session);});
      m_service.publish(resource);
    }

    m_webfileBlackList.insert("index.html");
  }


  SetupBackend::~SetupBackend()
  {}


  bool SetupBackend::aborted() const
  {
    return m_aborted;
  }


  void SetupBackend::_onStart()
  {
    std::thread spawnBrowserThread([this](){_spawnBrowser();});
    spawnBrowserThread.detach();
  }


  void SetupBackend::_spawnBrowser()
  {
    while (m_service.is_down()){
      std::this_thread::sleep_for(100ms);
    }
    
    std::stringstream serviceUrlStream;
    serviceUrlStream << "http://127.0.0.1:" << m_settings->get_port();
    std::string serviceURL = serviceUrlStream.str();
    Logger::log("Setup WebUI is ready and available at " + serviceURL);

    if(system(std::string("xdg-open " + serviceURL).c_str()) != 0){
      Logger::error("Failed to open browser");
    }
  }


  void SetupBackend::_getVersion(const SharedSession& session) const
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


  void SetupBackend::_finish(const SharedSession& session)
  {
    std::string response = "{}";
    session->close(restbed::OK, response, {
      {"Content-Length", std::to_string(response.size())},
      {"Content-Type", "application/json"}
    });

    stop();
  }


  void SetupBackend::_abort(const SharedSession& session)
  {
    std::string response = "{}";
    session->close(restbed::OK, response, {
      {"Content-Length", std::to_string(response.size())},
      {"Content-Type", "application/json"}
    });

    m_aborted = true;

    stop();
  }


  void SetupBackend::_autoDetectBridge(const SharedSession& session)
  {
    nlohmann::json jsonResponse = m_huenicornCore->autoDetectedBridge();
    std::string response = jsonResponse.dump();

    session->close(restbed::OK, response, {
      {"Content-Length", std::to_string(response.size())},
      {"Content-Type", "application/json"}
    });
  }

  
  void SetupBackend::_configFilePath(const SharedSession& session)
  {
    nlohmann::json jsonResponse = {{"configFilePath", m_huenicornCore->configFilePath()}};
    std::string response = jsonResponse.dump();

    session->close(restbed::OK, response, {
      {"Content-Length", std::to_string(response.size())},
      {"Content-Type", "application/json"}
    });
  }


  void SetupBackend::_validateBridgeAddress(const SharedSession& session)
  {
    const auto request = session->get_request();
    int contentLength = request->get_header("Content-Length", 0);

    session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
      std::string data(reinterpret_cast<const char*>(body.data()), body.size());
      nlohmann::json jsonBridgeAddressData = nlohmann::json::parse(data);

      std::string bridgeAddress = jsonBridgeAddressData.at("bridgeAddress");

      nlohmann::json jsonResponse = {{"succeeded", m_huenicornCore->validateBridgeAddress(bridgeAddress)}};

      std::string response = jsonResponse.dump();
      session->close(restbed::OK, response, {
        {"Content-Length", std::to_string(response.size())},
        {"Content-Type", "application/json"}
      });
    });
  }


  void SetupBackend::_validateCredentials(const SharedSession& session)
  {
    const auto request = session->get_request();
    int contentLength = request->get_header("Content-Length", 0);

    session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
      std::string data(reinterpret_cast<const char*>(body.data()), body.size());
      nlohmann::json jsonCredentials = nlohmann::json::parse(data);

      Credentials credentials(jsonCredentials.at("username"), jsonCredentials.at("clientkey"));

      nlohmann::json jsonResponse = {{"succeeded", m_huenicornCore->validateCredentials(credentials)}};

      std::string response = jsonResponse.dump();
      session->close(restbed::OK, response, {
        {"Content-Length", std::to_string(response.size())},
        {"Content-Type", "application/json"}
      });
    });
  }


  void SetupBackend::_registerNewUser(const SharedSession& session)
  {
    nlohmann::json jsonResponse = m_huenicornCore->registerNewUser();
    std::string response = jsonResponse.dump();
    session->close(restbed::OK, response, {
      {"Content-Length", std::to_string(response.size())},
      {"Content-Type", "application/json"}
    });
  }
}
