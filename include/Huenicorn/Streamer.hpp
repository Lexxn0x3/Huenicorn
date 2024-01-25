#pragma once

#include <string>
#include <vector>

#include <Huenicorn/Channel.hpp>
#include <Huenicorn/DtlsClient.hpp>


namespace Huenicorn
{
  static constexpr std::string PORT = "2100";

  /**
   * @brief Wrapper around UDP requests to submit color data to the bridge
   * 
   */
  class Streamer
  {
    struct HuestreamHeader
    {
      char protocolName[9] = {'H', 'u', 'e', 'S', 't', 'r', 'e', 'a', 'm'};
      char version[2] = {0x02, 0x00};
      char sequenceId = 0;
      char reserved1[2] = {0, 0};
      char colorSpace = 0x00;
      char reserved2 = 0;
      char entertainmentConfigurationId[36];


      /**
       * @brief Sets the colorSpace field
       * 
       * @param _colorSpace 0 for RGB, 1 for XY
       */
      void setColorSpace(char _colorSpace)
      {
        this->colorSpace = _colorSpace;
      }


      /**
       * @brief Sets the entertainmentConfigurationId field
       * 
       * @param _entertainmentConfigurationId Id of the entertainment configuration to set
       */
      void setEntertainmentConfigurationId(const std::string& _entertainmentConfigurationId)
      {
        for(int i = 0; const char& idChar : _entertainmentConfigurationId){
          entertainmentConfigurationId[i++] = idChar;
        }
      }
    };


    struct HuestreamPayload
    {
      char channelId;
      char colorData0[2];
      char colorData1[2];
      char colorData2[2];


      /**
       * @brief Sets the channel ID
       * 
       * @param _channelId ID of the channel
       */
      void setChannelId(char _channelId)
      {
        channelId = _channelId;
      }


      /**
       * @brief Sets the Red field value
       * 
       * @param red Red value
       */
      void setR(uint16_t red)
      {
        uint8_t a = static_cast<uint8_t>((red >> 8) & 0xff);
        uint8_t b = static_cast<uint8_t>(red & 0xff);
        colorData0[0] = a;
        colorData0[1] = b;
      }


      /**
       * @brief Sets the Green field value
       * 
       * @param green Green value
       */
      void setG(uint16_t green)
      {
        uint8_t a = static_cast<uint8_t>((green >> 8) & 0xff);
        uint8_t b = static_cast<uint8_t>(green & 0xff);
        colorData1[0] = a;
        colorData1[1] = b;
      }


      /**
       * @brief Sets the blue field value
       * 
       * @param blue Blue value
       */
      void setB(uint16_t blue)
      {
        uint8_t a = static_cast<uint8_t>((blue >> 8) & 0xff);
        uint8_t b = static_cast<uint8_t>(blue & 0xff);
        colorData2[0] = a;
        colorData2[1] = b;
      }
    };


  public:
    // Constructor
    /**
     * @brief Streamer constructor
     * 
     * @param credentials Hue bridge credentials
     * @param bridgeAddress Hue bridge address
     */
    Streamer(const Credentials& credentials, const std::string& bridgeAddress);


    // Setters
    /**
     * @brief Sets the entertainment configuration header field
     * 
     * @param entertainmentConfigurationId ID of the entertainment configuration to set to the request header
     */
    void setEntertainmentConfigurationId(const std::string& entertainmentConfigurationId);


    // Methods
    /**
     * @brief Submit the channels data to the stream
     * 
     * @param channels 
     */
    void streamChannels(const ChannelStreams& channels);


  private:
    // Attributes
    DtlsClient m_dtlsClient;
    HuestreamHeader m_header;
  };
}
