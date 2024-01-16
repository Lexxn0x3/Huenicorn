#pragma once

#include <string>
#include <vector>

#include <mbedtls/net_sockets.h>
#include <mbedtls/debug.h>
#include <mbedtls/ssl.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/error.h>
#include <mbedtls/timing.h>

#include <Huenicorn/Credentials.hpp>


namespace Huenicorn
{
  /**
   * @brief Wrapper around Mbed-TLS library to provide TLS v1.2 connection to Hue bridge
   * 
   */
  class DtlsClient
  {
    // Constants
    static constexpr std::string Hostname = "Hue";
    static constexpr unsigned HandshakeAttempts = 4;

  public:
    // Constructor / destructor
    /**
     * @brief DtlsClient constructor
     * 
     * @param credentials PSK credentials
     * @param address Address of the DTLS server
     * @param port Port of the DTLS server
     */
    DtlsClient(const Credentials& credentials, const std::string& address, const std::string& port);


    /**
     * @brief DtlsClient destructor
     * 
     */
    ~DtlsClient();

    /**
     * @brief Calls all the inner initializations
     * 
     */
    void init();


    /**
     * @brief Sends byte buffer to the server
     * 
     * @param requestBuffer Byte buffer to send to the server
     * @return true Sending was successful
     * @return false Sending failed
     */
    bool send(const std::vector<char>& requestBuffer);

  private:
    // Private methods
    /**
     * @brief Initializes MberdTLS objects
     * 
     */
    void _initMembers();


    /**
     * @brief Initializes debug messages for Mbed-TLS
     * 
     */
    void _initDebug();


    /**
     * @brief Initializes the random number generator
     * 
     */
    void _initRNG();


    /**
     * @brief Initializes the connection
     * 
     */
    void _initConnection();


    /**
     * @brief Initializes the SSL layer with PSK
     * 
     */
    void _initSSL();


    /**
     * @brief Execute handshake
     * 
     */
    void _handshake();


    /**
     * @brief Free all the allocated MbedTLS members
     * 
     */
    void _deallocate();

    // Attributes
    const Credentials m_credentials;
    const std::string m_address;
    const std::string m_port;

    std::vector<int> m_ciphers;

    // MbedTLS
    mbedtls_net_context m_serverFd;
    mbedtls_entropy_context m_entropy;
    mbedtls_ctr_drbg_context m_ctrDrbg;
    mbedtls_ssl_context m_ssl;
    mbedtls_ssl_config m_conf;
    mbedtls_x509_crt m_cacert;
    mbedtls_timing_delay_context m_timer;
  };
}
