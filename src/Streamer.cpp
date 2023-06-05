#include <Huenicorn/Streamer.hpp>

#include <iostream>

using namespace glm;
using namespace std;


namespace Huenicorn
{
  Streamer::Streamer(const Credentials& credentials, const std::string& bridgeAddress):
    m_dtlsClient(credentials, bridgeAddress, PORT)
  {
    try{
      m_dtlsClient.init();
    }
    catch(const std::exception& e){
      cout << e.what();
    }
  }


  void Streamer::setEntertainmentConfigurationId(const std::string& entertainmentConfigurationId)
  {
    m_header.setEntertainmentConfigurationId(entertainmentConfigurationId);
  }


  void Streamer::streamChannels(const ChannelStreams& channels)
  {
    vector<char> requestBuffer;
    requestBuffer.insert(requestBuffer.end(), reinterpret_cast<char*>(&m_header), reinterpret_cast<char*>(&m_header) + sizeof(HuestreamHeader));

    for(const auto channel : channels){
      HuestreamPayload payload;
      payload.setChannelId(channel.id);
      payload.setR(static_cast<uint16_t>(0xffff * channel.colorData[0]));
      payload.setG(static_cast<uint16_t>(0xffff * channel.colorData[1]));
      payload.setB(static_cast<uint16_t>(0xffff * channel.colorData[2]));

      requestBuffer.insert(requestBuffer.end(), reinterpret_cast<char*>(&payload), reinterpret_cast<char*>(&payload) + sizeof(HuestreamPayload));
    }

    m_dtlsClient.send(requestBuffer);
    m_header.sequenceId++;
  }
}
