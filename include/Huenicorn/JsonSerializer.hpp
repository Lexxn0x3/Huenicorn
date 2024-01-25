#pragma once

#include <nlohmann/json.hpp>

#include <Huenicorn/ApiTools.hpp>
#include <Huenicorn/Channel.hpp>
#include <Huenicorn/Credentials.hpp>
#include <Huenicorn/Device.hpp>
#include <Huenicorn/UV.hpp>


namespace Huenicorn
{
  /**
   * @brief Provides json serialization functions for data structures
   * 
   */
  namespace JsonSerializer
  {
    /**
     * @brief Serializes Credentials to JSON
     * 
     * @param credentials Credentials data to serialize
     * @return nlohmann::json Serialized credentials
     */
    nlohmann::json serialize(const Credentials& credentials);


    /**
     * @brief Serializes EntertainmentConfiguration to JSON
     * 
     * @param entertainmentConfiguration data to serialize
     * @return nlohmann::json Serialized entertainment configuration
     */
    nlohmann::json serialize(const EntertainmentConfiguration& entertainmentConfiguration);


    /**
     * @brief Serializes EntertainmentConfigurations to JSON
     * 
     * @param entertainmentConfigurations entertainmentConfigurations to serialize
     * @return nlohmann::json Serialized entertainmentConfigurations
     */
    nlohmann::json serialize(const EntertainmentConfigurations& entertainmentConfigurations);


    /**
     * @brief Serializes UVs to JSON
     * 
     * @param uvs UVs to serialize
     * @return nlohmann::json Serialized UVs
     */
    nlohmann::json serialize(const UVs& uvs);


    /**
     * @brief Serializes Device to JSON
     * 
     * @param device Device to serialize
     * @return nlohmann::json Serialized device
     */
    nlohmann::json serialize(const Device& device);


    /**
     * @brief Serializes Devices to JSON
     * 
     * @param devices Devices to serialize
     * @return nlohmann::json Serialized devices
     */
    nlohmann::json serialize(const Devices& devices);


    /**
     * @brief Serializes Channel to JSON
     * 
     * @param channel Channel to serialize
     * @return nlohmann::json Serialized channel
     */
    nlohmann::json serialize(const Channel& channel);


    /**
     * @brief Serializes Channels to JSON
     * 
     * @param channels Channels to serialize
     * @return nlohmann::json Serialized channels
     */
    nlohmann::json serialize(const Channels& channels);


    /**
     * @brief Serializes MembersIds to JSON
     * 
     * @param membersIds MembersIds to serialize
     * @return nlohmann::json Serialized membersIds
     */
    nlohmann::json serialize(const MembersIds& membersIds);


    /**
     * @brief Serialize a List of devices
     * 
     * @param devices Device list to serialize
     * @return nlohmann::json Serialized device list
     */
    nlohmann::json serialize(const std::vector<Device>& devices);


    /**
     * @brief Serializes ChannelsMembers to JSON
     * 
     * @param channelsMembers Channels members to serialize
     * @return nlohmann::json Serialized channels members
     */
    nlohmann::json serialize(const ChannelsMembers& channelsMembers);
  }
}
