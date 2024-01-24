#pragma once

#include <string>

namespace Huenicorn
{
  // Forward declaration for shorthand
  namespace Logger{enum class LogLevel;};
  using Logger::LogLevel;

  namespace Logger
  {
    enum class LogLevel 
    {
      Message,
      Warning,
      Error,
      Debug,
    };

    void log(LogLevel logLevel, const std::string& message);

    void log(const std::string& logMessage);

    void warn(const std::string& warningMessage);

    void error(const std::string& errorMessage);

    void debug(const std::string& debugMessage);
  }
}
