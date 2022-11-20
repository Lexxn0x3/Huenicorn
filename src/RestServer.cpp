#include <FreenSync/RestServer.hpp>

#include <iostream>
#include <fstream>

#include <nlohmann/json.hpp>

#include <FreenSync/FreenSync.hpp>

using namespace nlohmann;
using namespace std;

RestServer::RestServer(FreenSync* freenSync):
m_freenSync(freenSync),
m_webroot("webroot")
{
  m_settings = make_shared<restbed::Settings>();
  m_settings->set_default_headers({
    {"Connection", "close"},
    {"Content-Type", "text/plain"},
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
    resource->set_path("/allLights");
    resource->set_method_handler("GET", [this](SharedSession session){_getAllLights(session);});
    m_service.publish(resource);
  }

  {
    auto resource = make_shared<restbed::Resource>();
    resource->set_path("/screen");
    resource->set_method_handler("GET", [this](SharedSession session){_getScreen(session);});
    m_service.publish(resource);
  }

  {
    auto resource = make_shared<restbed::Resource>();
    resource->set_path("/setLightUVs/{lightId: .+}");
    resource->set_method_handler("PUT", [this](SharedSession session){_setLightUVs(session);});
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
    resource->set_path("/saveProfile");
    resource->set_method_handler("POST", [this](SharedSession session){_saveProfile(session);});
    m_service.publish(resource);
  }
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


bool RestServer::stop()
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
    const auto& request = session->get_request();

    json jsonLights = json::array();
    for(const auto& [_, light] : m_freenSync->availableLights()){
      jsonLights.push_back(
      {
        {"id", light.id},
        {"name", light.name},
        {"productName", light.productName}
      });
    }

    string response = jsonLights.dump();
    
    session->close(restbed::OK, response, {{"Content-Length", std::to_string(response.size())}});
  });
}


void RestServer::_getSyncedLights(const SharedSession& session) const
{
  const auto request = session->get_request();
  int contentLength = request->get_header("Content-Length", 0);

  session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
    const auto& request = session->get_request();

    json jsonLights = json::array();
    for(const auto& [_, light] : m_freenSync->syncedLights()){
      jsonLights.push_back(light->serialize());
    }

    string response = jsonLights.dump();
    
    session->close(restbed::OK, response, {{"Content-Length", std::to_string(response.size())}});
  });
}


void RestServer::_getAllLights(const SharedSession& session) const
{
  const auto request = session->get_request();
  int contentLength = request->get_header("Content-Length", 0);

  session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
    const auto& request = session->get_request();

    json jsonAvailableLights = json::array();
    for(const auto& [_, light] : m_freenSync->availableLights()){
      jsonAvailableLights.push_back({
        {"id", light.id},
        {"name", light.name},
        {"productName", light.productName},
      });
    }

    json jsonSyncedLights = json::array();
    for(const auto& [_, light] : m_freenSync->syncedLights()){
      jsonSyncedLights.push_back(light->serialize());
    }

    json jsonAllLights = json::object();
    jsonAllLights["available"] = jsonAvailableLights;
    jsonAllLights["synced"] = jsonSyncedLights;

    string response = jsonAllLights.dump();

    session->close(restbed::OK, response, {{"Content-Length", std::to_string(response.size())}});
  });
}


void RestServer::_getScreen(const SharedSession& session) const
{
  const auto request = session->get_request();
  int contentLength = request->get_header("Content-Length", 0);

  session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){
    glm::vec2 screenResolution = m_freenSync->screenResolution();
    json jsonScreen{
      {"x", screenResolution.x},
      {"y", screenResolution.y}
    };

    string response = jsonScreen.dump();
    
    session->close(restbed::OK, response, {{"Content-Length", std::to_string(response.size())}});
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


void RestServer::_setLightUVs(const SharedSession& session) const
{
  const auto request = session->get_request();
  int contentLength = request->get_header("Content-Length", 0);

  session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){

    string data(reinterpret_cast<const char*>(body.data()), body.size());

    const auto& request = session->get_request();
    string lightId = request->get_path_parameter("lightId");

    json jsonUVs = json::parse(data);

    float uvAx = jsonUVs.at("uvA").at("x");
    float uvAy = jsonUVs.at("uvA").at("y");
    float uvBx = jsonUVs.at("uvB").at("x");
    float uvBy = jsonUVs.at("uvB").at("y");

    const auto& clampedUVs = m_freenSync->syncedLights().at(lightId)->setUVs({{uvAx, uvAy}, {uvBx, uvBy}});

    json jsonResponse = {
      {
        {"uvA", {{"x", clampedUVs.first.x}, {"y", clampedUVs.first.y}}},
        {"uvB", {{"x", clampedUVs.second.x}, {"y", clampedUVs.second.y}}}
      }
    };

    string response = jsonResponse.dump();
    
    session->close(restbed::OK, response, {{"Content-Length", std::to_string(response.size())}});
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
    const auto& availableLights = m_freenSync->availableLights();
    if(availableLights.find(lightId) == availableLights.end()){
      string response = json{
        {"status", false},
        {"error", "key not found"}
      }.dump();
      session->close(restbed::OK, response, {{"Content-Length", std::to_string(response.size())}});
      return;
    }

    json jsonResponse = json::object();
    bool succeeded = m_freenSync->addSyncedLight(lightId) != nullptr;
    if(succeeded){
      jsonResponse["newSyncedLightId"] = lightId;
    }

    jsonResponse["status"] = succeeded;
    jsonResponse["syncedLights"] = json::array();

    for(const auto& [_, syncedLight] : m_freenSync->syncedLights()){
      jsonResponse["syncedLights"].push_back(syncedLight->serialize());
    }

    string response = jsonResponse.dump();
    session->close(restbed::OK, response, {{"Content-Length", std::to_string(response.size())}});
  });
}


void RestServer::_saveProfile(const SharedSession& session) const
{
  const auto request = session->get_request();
  int contentLength = request->get_header("Content-Length", 0);

  session->fetch(contentLength, [this](const SharedSession& session, const restbed::Bytes& body){

    m_freenSync->saveProfile();

    json jsonResponse = {
      "status", true
    };

    string response = jsonResponse.dump();
    session->close(restbed::OK, response, {{"Content-Length", std::to_string(response.size())}});
  });
}
