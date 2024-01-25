#pragma once

#include <filesystem>
#include <fstream>
#include <future>
#include <optional>
#include <unordered_set>

#include <restbed>


namespace Huenicorn
{
  /**
   * @brief Abstract class to implement for REST service
   * 
   */
  class IRestServer
  {
  protected:
    // Type definitions
    using SharedSession = std::shared_ptr<restbed::Session>;

  public:
    // Constructor / Destructor
    /**
     * @brief IRestServer constructor
     * 
     * @param webRoot Path to the web root directory
     */
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
  
      {
        auto resource = std::make_shared<restbed::Resource>();
        resource->set_path("/version");
        resource->set_method_handler("GET", [this](SharedSession session){_getVersion(session);});
        m_service.publish(resource);
      }
    }


    /**
     * @brief IRestServer destructor
     * 
     */
    virtual ~IRestServer(){}


    // Getters
    /**
     * @brief Returns whether the REST server is running or not
     * 
     * @return true REST server is running
     * @return false REST server is not running
     */
    bool running() const
    {
      return m_service.is_up();
    }


    // Methods
    /**
     * @brief Starts the REST server and triggers _onStart event
     * 
     * @param port Listening port of the REST server
     * @param boundBackendIP IP to bind the backend to
     * @return true REST server started successfully
     * @return false REST server is already running
     */
    bool start(unsigned port, const std::string& boundBackendIP)
    {
      if(running()){
        return false;
      }

      m_settings->set_port(port);
      m_settings->set_bind_address(boundBackendIP);

      _onStart();
      m_service.start(m_settings);

      return true;
    }


    /**
     * @brief (Overload) Starts the REST server and triggers _onStart event
     * 
     * @param port Listening port of the REST server
     * @param boundBackendIP IP to bind the backend to
     * @param readyPromise Promise to notify readyness to the parent thread
     * @return true REST server started successfully
     * @return false REST server is already running
     */
    bool start(unsigned port, const std::string& boundBackendIP, std::promise<bool>&& readyPromise)
    {
      m_readyWebUIPromise.emplace(std::move(readyPromise));
      return start(port, boundBackendIP);
    }


    /**
     * @brief Stops the REST server
     * 
     * @return true 
     * @return false 
     */
    bool stop()
    {
      return _stop();
    }

  private:
    // Private method


    /**
     * @brief Stops REST server and triggers _onStop event
     * 
     * @return true REST server stopped successfully
     * @return false REST server was not running
     */
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
    // Protected methods

    /**
     * @brief Overridable routine triggering at server start
     * 
     */
    virtual void _onStart(){}


    /**
     * @brief Overridable routine triggering at server stop
     * 
     */
    virtual void _onStop(){}


    // Handlers
    /**
     * @brief Returns the version of the backend project
     * 
     * @param session Pending HTTP connection
     */
    virtual void _getVersion(const SharedSession& session) const = 0;


    /**
     * @brief Web filesystem handler
     * 
     * @param session Pending HTTP connection
     */
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

      std::ifstream webFile(webFileFullPath);
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
    std::optional<std::promise<bool>> m_readyWebUIPromise;
  };
}
