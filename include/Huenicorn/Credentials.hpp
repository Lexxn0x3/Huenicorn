#pragma once

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>


namespace Huenicorn
{
  std::vector<unsigned char> hexStringToBytes(const std::string& hexString);
  std::vector<unsigned char> stringToBytes(const std::string& string);


  /**
   * @brief Wrapper around Hue bridge user authentication data
   * 
   */
  class Credentials
  {
  public:
    // Constructor
    /**
     * @brief Credentials constructor
     * 
     * @param username Hue bridge username
     * @param clientkey Hue bridge clientkey
     */
    Credentials(const std::string& username, const std::string& clientkey);


    // Getters
    /**
     * @brief Returns the registered Hue bridge username
     * 
     * @return const std::string& Hue bridge username
     */
    const std::string& username() const;


    /**
     * @brief Returns the registered Hue bridge clientkey
     * 
     * @return const std::string&  Hue bridge clientkey
     */
    const std::string& clientkey() const;


    /**
     * @brief Helper to get byte representation of username
     * 
     * @return std::vector<unsigned char> username as bytes
     */
    std::vector<unsigned char> usernameBytes() const;


    /**
     * @brief Helper to get byte representation of clientkey
     * 
     * @return std::vector<unsigned char> clientkey as bytes
     */
    std::vector<unsigned char> clientkeyBytes() const;


  private:
    // Attributes
    std::string m_username;
    std::string m_clientkey;
  };
}
