#pragma once

#include <Huenicorn/IRestServer.hpp>

#include <filesystem>


namespace Huenicorn
{
  class HuenicornCore;


  /**
   * @brief REST service handling requests for light management
   * 
   */
  class WebUIBackend : public IRestServer
  {
    // Type definitions
    using SharedSession = std::shared_ptr<restbed::Session>;

  public:
    // Constructor
    /**
     * @brief WebUIBackend constructor
     * 
     * @param huenicornCore Pointer to Huenicorn core
     */
    WebUIBackend(HuenicornCore* huenicornCore);


  protected:
    // Overrides
    /**
     * @brief Overriden routine to prompt message
     * 
     */
    void _onStart() override;


    // Handlers
    /**
     * @brief Returns the version of the backend project
     * 
     * @param session Pending HTTP connection
     */
    void _getVersion(const SharedSession& session) const override;


  private:
    /**
     * @brief Handler to check the availability of the webUI
     * 
     * @param session Pending HTTP connection
     */
    void _getWebUIStatus(const SharedSession& session) const;


    /**
     * @brief Handler to get available entertainment configurations
     * 
     * @param session Pending HTTP connection
     */
    void _getEntertainmentConfigurations(const SharedSession& session) const;


    /**
     * @brief Handler to get channel data
     * 
     * @param session Pending HTTP connection
     */
    void _getChannel(const SharedSession& session) const;


    /**
     * @brief Handlerto get the channels data
     * 
     * @param session Pending HTTP connection
     */
    void _getChannels(const SharedSession& session) const;


    /**
     * @brief Handler to get Channels members
     * 
     * @param session Pending HTTP connection
     */
    void _getChannelsMembers(const SharedSession& session) const;


    /**
     * @brief Handler to get the display informations
     * 
     * @param session Pending HTTP connection
     */
    void _getDisplayInfo(const SharedSession& session) const;


    /**
     * @brief Handler to get the interpolation informations
     * 
     * @param session Pending HTTP connection
     */
    void _getInterpolationInfo(const SharedSession& session) const;


    /**
     * @brief Handler to set the current entertainment configuration
     * 
     * @param session Pending HTTP connection
     */
    void _setEntertainmentConfiguration(const SharedSession& session) const;


    /**
     * @brief Handler to set the UV coordinates for a given channel
     * 
     * @param session Pending HTTP connection
     */
    void _setChannelUV(const SharedSession& session) const;


    /**
     * @brief Handler to set the gamma factor for a given channel
     * 
     * @param session Pending HTTP connection
     */
    void _setChannelGammaFactor(const SharedSession& session) const;


    /**
     * @brief Handler to set the image subsample width
     * 
     * @param session Pending HTTP connection
     */
    void _setSubsampleWidth(const SharedSession& session) const;


    /**
     * @brief Handler to set the streaming refresh rate
     * 
     * @param session Pending HTTP connection
     */
    void _setRefreshRate(const SharedSession& session) const;


    /**
     * @brief Handler to set the interpolation type
     * 
     * @param session Pending HTTP connection
     */
    void _setInterpolation(const SharedSession& session) const;


    /**
     * @brief Handler to set the channel streaming state
     * 
     * @param session Pending HTTP connection
     */
    void _setChannelActivity(const SharedSession& session) const;


    /**
     * @brief Handler to save the user profile
     * 
     * @param session Pending HTTP connection
     */
    void _saveProfile(const SharedSession& session) const;


    /**
     * @brief Handler to stop Huenicorn
     * 
     * @param session Pending HTTP connection
     */
    void _stop(const SharedSession& session) const;


    // Attributes
    HuenicornCore* m_huenicornCore;
  };
}
