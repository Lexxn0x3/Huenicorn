#include <Huenicorn/Logger.hpp>

#include <iostream>

namespace Huenicorn
{
  namespace Logger
  {
    void log(LogLevel logLevel, const std::string message)
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
          std::cerr << "[Debug] : " << message << "\n";
          break;
  #endif // NDEBUG

        default:
          break;
      }
    }


    void log(const std::string message)
    {
      log(LogLevel::Message, message);
    }
  }
}
