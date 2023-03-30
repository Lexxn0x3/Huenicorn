#pragma once

#include <filesystem>
#include <fstream>
#include <unordered_set>

#include <restbed>


namespace Huenicorn
{
  class IRestServer
  {
  protected:
    // Type definitions
    using SharedSession = std::shared_ptr<restbed::Session>;

  public:
    // Constructor / Destructor
    IRestServer(const std::filesystem::path& webRoot):
    m_webroot(webRoot)
    {
      m_settings = std::make_shared<restbed::Settings>();
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
        auto resource = std::make_shared<restbed::Resource>();
        resource->set_path("/");
        resource->set_method_handler("GET", [this](SharedSession session){_getWebFile(session);});
        m_service.publish(resource);
      }

      {
        auto resource = std::make_shared<restbed::Resource>();
        resource->set_path("/{webFileName: [a-zA-Z0-9]+(\\.)[a-zA-Z0-9]+}");
        resource->set_method_handler("GET", [this](SharedSession session){_getWebFile(session);});
        m_service.publish(resource);
      }
    }


    virtual ~IRestServer(){}


    // Getters
    bool running() const
    {
      return m_service.is_up();
    }


    // Methods
    bool start(unsigned port)
    {
      if(running()){
        return false;
      }

      m_settings->set_port(port);

      _onStart();

      m_service.start(m_settings);

      return true;
    }


    bool stop()
    {
      return _stop();
    }

  private:
    bool _stop()
    {
      if(!running()){
        return false;
      }

      m_service.stop();

      _onStop();

      return true;
    }

  protected:
    virtual void _onStart(){}
    virtual void _onStop(){}


    // Handlers
    void _getWebFile(const SharedSession& session) const
    {
      const auto request = session->get_request();
      std::filesystem::path webFileName = request->get_path_parameter("webfileName");

      if(webFileName == ""){
        webFileName = m_indexFile;
      }

      std::filesystem::path webFileFullPath = m_webroot / webFileName;

      if(!std::filesystem::exists(m_webroot)){
        std::string response = "<h1>Error : Could not locate webroot</h1><p>Make sure that the webroot directory figures in the current working directory</p>";
        std::string contentType = "text/html";

        std::multimap<std::string, std::string> headers{
          {"Content-Length", std::to_string(response.size())},
          {"Content-Type", contentType}
        };

        session->close(restbed::OK, response, headers);
        return;
      }

      if(!std::filesystem::exists(webFileFullPath) || m_webfileBlackList.contains(webFileName)){
        webFileName = "404.html";
        webFileFullPath = m_webroot / webFileName;
      }

      std::string extension = webFileName.extension().string();
      std::string contentType = "text/plain";

      if(m_contentTypes.find(extension) != m_contentTypes.end()){
        contentType = m_contentTypes.at(extension);
      }

      std::fstream webFile(webFileFullPath);
      std::string response = std::string(std::istreambuf_iterator<char>(webFile), std::istreambuf_iterator<char>());

      std::multimap<std::string, std::string> headers{
        {"Content-Length", std::to_string(response.size())},
        {"Content-Type", contentType}
      };

      session->close(restbed::OK, response, headers);
    }

    // Attributes
    std::shared_ptr<restbed::Settings> m_settings;
    restbed::Service m_service;
    const std::filesystem::path m_webroot;
    std::unordered_map<std::string, std::string> m_contentTypes;
    std::string m_indexFile{"index.html"};
    std::unordered_set<std::string> m_webfileBlackList;
  };
}
