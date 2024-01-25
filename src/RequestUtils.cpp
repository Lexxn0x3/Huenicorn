#include <Huenicorn/RequestUtils.hpp>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Easy.hpp>

#include <Huenicorn/Logger.hpp>


namespace Huenicorn
{
  namespace RequestUtils
  {
    nlohmann::json sendRequest(const std::string& url, const std::string& method, const std::string& body, const Headers& headers)
    {
      nlohmann::json jsonBody = nlohmann::json::object();
      curlpp::Cleanup cleaner;
      curlpp::Easy request;

      request.setOpt(new curlpp::options::Url(url));
      request.setOpt(new curlpp::options::CustomRequest{method});
      request.setOpt(new curlpp::options::Timeout(1));

      if(body.size() > 0){
        request.setOpt(new curlpp::options::PostFields(body));
        request.setOpt(new curlpp::options::PostFieldSize(body.length()));
      }

      if(headers.size() > 0){
        // Disable ssl checks for the sake of getting data without trouble
        request.setOpt(new curlpp::options::SslVerifyPeer(false));
        request.setOpt(new curlpp::options::SslVerifyHost(false));

        std::list<std::string> concatenatedHeaders;
        for(const auto& header : headers){
          std::string concat = header.first + ": " + header.second;
          concatenatedHeaders.push_back(concat);
        }
        
        request.setOpt(new curlpp::options::HttpHeader(concatenatedHeaders));
      }

      std::ostringstream response;
      request.setOpt(new curlpp::options::WriteStream(&response));

      nlohmann::json jsonResponse = {};
      try{
        request.perform();
        jsonResponse = nlohmann::json::parse(response.str());
      }
      catch(const curlpp::LibcurlRuntimeError& e){
        Logger::error(e.what());
      }
      catch(const nlohmann::json::exception& e){
        Logger::error(e.what());
      }

      return jsonResponse;
    }
  }
}
