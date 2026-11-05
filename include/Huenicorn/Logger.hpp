#pragma once

#include <iostream>
#include <sstream>

namespace Huenicorn
{
  namespace Logger
  {
    enum class LogLevel
    {
      Message,
      Warning,
      Error,
      Debug,
    };

    void logHelper(std::ostringstream&);

    template<typename T, typename... Args>
    void logHelper(std::ostringstream& oss, T&& t, Args&&... args)
    {
      oss << std::forward<T>(t);
      logHelper(oss, std::forward<Args>(args)...);
    }


    template<typename T>
    void logHelper(std::ostringstream& oss, T&& t)
    {
      oss << std::forward<T>(t);
    }


    template<typename... Args>
    void log(LogLevel logLevel, Args&&... args)
    {
      std::ostringstream oss;
      logHelper(oss, std::forward<Args>(args)...);

      switch(logLevel)
      {
        case LogLevel::Message:
          std::cout << oss.str() << "\n" << std::flush;
          break;

        case LogLevel::Warning:
          std::cout << "[Warning] : " << oss.str() << "\n" << std::flush;
          break;

        case LogLevel::Error:
          std::cerr << "[Error] : " << oss.str() << "\n" << std::flush;
          break;

  #ifndef NDEBUG
        case LogLevel::Debug:
          std::cout << "[Debug] : " << oss.str() << "\n" << std::flush;
          break;
  #endif // NDEBUG

        default:
          break;
      }
    }


    template<typename... Args>
    void log(Args&&... args)
    {
      log(LogLevel::Message, std::forward<Args>(args)...);
    }


    template<typename... Args>
    void warn(Args&&... args)
    {
      log(LogLevel::Warning, std::forward<Args>(args)...);
    }


    template<typename... Args>
    void error(Args&&... args)
    {
      log(LogLevel::Error, std::forward<Args>(args)...);
    }


    template<typename... Args>
    void debug(Args&&... args)
    {
      log(LogLevel::Debug, std::forward<Args>(args)...);
    }
  }
}
