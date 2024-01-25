#include <Huenicorn/Credentials.hpp>


namespace Huenicorn
{
  std::vector<unsigned char> hexStringToBytes(const std::string& hexString)
  {
    if(hexString.size() % 2 != 0){
      throw std::runtime_error("Wrong hex string length");
    }

    std::vector<unsigned char> bytes;
    bytes.resize(hexString.size() / 2);

    std::stringstream converter;
    int byte;
    for(int i = 0; auto& b : bytes){
      converter << std::hex << hexString.substr(i, 2);
      converter >> byte;
      b = byte & 0xFF;
      converter.clear();
      i += 2;
    }

    return bytes;
  }


  std::vector<unsigned char> stringToBytes(const std::string& string)
  {
    std::vector<unsigned char> bytes;
    for(unsigned char c : string){
      bytes.push_back(static_cast<unsigned char>(c));
    }

    return bytes;
  }

  
  Credentials::Credentials(const std::string& username, const std::string& clientkey):
  m_username(username),
  m_clientkey(clientkey)
  {}


  const std::string& Credentials::username() const
  {
    return m_username;
  }


  const std::string& Credentials::clientkey() const
  {
    return m_clientkey;
  }


  std::vector<unsigned char> Credentials::usernameBytes() const
  {
    return stringToBytes(m_username);
  }


  std::vector<unsigned char> Credentials::clientkeyBytes() const
  {
    return hexStringToBytes(m_clientkey);
  }
}
