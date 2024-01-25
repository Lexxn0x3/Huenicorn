#include <Huenicorn/Streamer.hpp>

#include <Huenicorn/Logger.hpp>


namespace Huenicorn
{
  Streamer::Streamer(const Credentials& credentials, const std::string& bridgeAddress):
    m_dtlsClient(credentials, bridgeAddress, PORT)
  {
    try{
      m_dtlsClient.init();
    }
    catch(const std::exception& e){
      Logger::error(e.what());
    }
  }


  void Streamer::setEntertainmentConfigurationId(const std::string& entertainmentConfigurationId)
  {
    m_header.setEntertainmentConfigurationId(entertainmentConfigurationId);
  }


  void Streamer::streamChannels(const ChannelStreams& channels)
  {
    std::vector<char> requestBuffer;
    requestBuffer.insert(requestBuffer.end(), reinterpret_cast<char*>(&m_header), reinterpret_cast<char*>(&m_header) + sizeof(HuestreamHeader));

    for(const auto channel : channels){
      HuestreamPayload payload;
      payload.setChannelId(channel.id);
      payload.setR(static_cast<uint16_t>(0xffff * channel.r));
      payload.setG(static_cast<uint16_t>(0xffff * channel.g));
      payload.setB(static_cast<uint16_t>(0xffff * channel.b));

      requestBuffer.insert(requestBuffer.end(), reinterpret_cast<char*>(&payload), reinterpret_cast<char*>(&payload) + sizeof(HuestreamPayload));
    }

    m_dtlsClient.send(requestBuffer);
    m_header.sequenceId++;
  }
}
