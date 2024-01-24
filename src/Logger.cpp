#include <Huenicorn/Logger.hpp>

#include <iostream>

namespace Huenicorn
{
  namespace Logger
  {
    void log(LogLevel logLevel, const std::string& message)
    {
      switch(logLevel)
      {
        case LogLevel::Message:
          std::cout << message << "\n";
          break;

        case LogLevel::Warning:
          std::cout << "[Warning] : " << message << "\n";
          break;

        case LogLevel::Error:
          std::cerr << "[Error] : " << message << "\n";
          break;

  #ifndef NDEBUG
        case LogLevel::Debug:
          std::cout << "[Debug] : " << message << "\n";
          break;
  #endif // NDEBUG

        default:
          break;
      }
    }


    void log(const std::string& logMessage)
    {
      log(LogLevel::Message, logMessage);
    }


    void warn(const std::string& warningMessage)
    {
      log(LogLevel::Warning, warningMessage);
    }


    void error(const std::string& errorMessage)
    {
      log(LogLevel::Error, errorMessage);
    }


    void debug(const std::string& debugMessage)
    {
      log(LogLevel::Debug, debugMessage);
    }
  }
}
