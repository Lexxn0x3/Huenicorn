#include <Huenicorn/DtlsClient.hpp>

#include <stdexcept>
#include <iostream>
#include <thread>
#include <sstream>

// Begin MbedTLS part
#ifndef MBEDTLS_CONFIG_FILE
#include <mbedtls/config.h>
#else
#include MBEDTLS_CONFIG_FILE
#endif

#ifdef MBEDTLS_PLATFORM_C
#include <mbedtls/platform.h>
#endif
// End MbedTLS part

using namespace std;

namespace Huenicorn
{
  std::vector<unsigned char> fromHex(const std::string& hexString)
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


  DtlsClient::DtlsClient(const std::string& username, const std::string& clientkey, const std::string& address, const std::string& port):
    m_username(username),
    m_clientkey(clientkey),
    m_address(address),
    m_port(port)
  {
  }


  DtlsClient::~DtlsClient()
  {
    _deallocate();
  }


  void DtlsClient::init()
  {
    mbedtls_debug_set_threshold(4);

    _initMembers();
    _initRNG();
    _initConnection();
    _initSSL();
    _handshake();
  }


  bool DtlsClient::send(const std::vector<char>& requestBuffer)
  {
    int result = mbedtls_ssl_write(&m_ssl, (unsigned char *)requestBuffer.data(), requestBuffer.size());
    bool succeeded = result ==  MBEDTLS_ERR_SSL_WANT_READ ||
                result == MBEDTLS_ERR_SSL_WANT_WRITE ||
                result >= 0;
    return succeeded;
  }


  void DtlsClient::_initMembers()
  {
    mbedtls_net_init(&m_serverFd);
    mbedtls_ssl_init(&m_ssl);
    mbedtls_ssl_config_init(&m_conf);
    mbedtls_x509_crt_init(&m_cacert);
    mbedtls_ctr_drbg_init(&m_ctrDrbg);
  }


  void DtlsClient::_initRNG()
  {
    string pers = "dtls_client";
    mbedtls_entropy_init(&m_entropy);
    int result = mbedtls_ctr_drbg_seed(
      &m_ctrDrbg,
      mbedtls_entropy_func,
      &m_entropy,
      (const unsigned char*)pers.data(),
      pers.length()
    );

    if(result != 0){
      throw runtime_error("mbedtls_ctr_drbg_seed returned: " + result);
    }
  }


  void DtlsClient::_initConnection()
  {
    cout << "Connecting to udp " << m_address << ":" << m_port << endl;

    int result = mbedtls_net_connect(
      &m_serverFd,
      m_address.c_str(),
      m_port.c_str(),
      MBEDTLS_NET_PROTO_UDP
    );

    if(result != 0){
      throw runtime_error("mbedtls_net_connect failed with code: " + result);
    }
  }


  void DtlsClient::_initSSL()
  {
    cout << "Setting up the DTLS structure..." << endl;
    cout << "username: " << m_username << endl;
    cout << "clientkey: " << m_clientkey << endl;

    std::vector pskRawArray = fromHex(m_clientkey);
    std::vector pskIdRawArray = stringToBytes(m_username);

    int result = mbedtls_ssl_config_defaults(
      &m_conf,
      MBEDTLS_SSL_IS_CLIENT,
      MBEDTLS_SSL_TRANSPORT_DATAGRAM,
      MBEDTLS_SSL_PRESET_DEFAULT
    );

    if(result != 0){
      throw runtime_error("mbedtls_ssl_config_defaults failed with code: " + result);
    }

    mbedtls_ssl_conf_authmode(&m_conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_ca_chain(&m_conf, &m_cacert, NULL);
    mbedtls_ssl_conf_rng(&m_conf, mbedtls_ctr_drbg_random, &m_ctrDrbg);

    result = mbedtls_ssl_setup(&m_ssl, &m_conf);

    if(result != 0){
      throw runtime_error("mbedtls_ssl_setup failed with code: " + result);
    }

    result = mbedtls_ssl_conf_psk(
      &m_conf,
      (const unsigned char*)pskRawArray.data(),
      pskRawArray.size() * sizeof(unsigned char),
      (const unsigned char*)pskIdRawArray.data(),
      pskIdRawArray.size() * sizeof(unsigned char)
    );

    if(result != 0){
      throw runtime_error("mbedtls_ssl_conf_psk failed with code: " + result);
    }

    int ciphers[2];
    ciphers[0] = MBEDTLS_TLS_PSK_WITH_AES_128_GCM_SHA256;
    ciphers[1] = 0;
    mbedtls_ssl_conf_ciphersuites(&m_conf, ciphers);

    result = mbedtls_ssl_set_hostname(&m_ssl, Hostname.c_str());

    if(result != 0){
      throw runtime_error("mbedtls_ssl_set_hostname failed with code: " + result);
    }

    mbedtls_ssl_set_bio(
      &m_ssl,
      &m_serverFd,
      mbedtls_net_send,
      mbedtls_net_recv,
      mbedtls_net_recv_timeout
    );

    mbedtls_ssl_set_timer_cb(
      &m_ssl,
      &m_timer,
      mbedtls_timing_set_delay,
      mbedtls_timing_get_delay
    );
  }


  void DtlsClient::_handshake()
  {
    cout << "Performing the DTLS handshake..." << endl;

    int result;
    for(int attempt = 0; attempt < 4; ++attempt){
      cout << "handshake attempt" << attempt << endl;
      mbedtls_ssl_conf_handshake_timeout(&m_conf, 400, 1000);
      do{
        result = mbedtls_ssl_handshake(&m_ssl);
      }
      while(result == MBEDTLS_ERR_SSL_WANT_READ || result == MBEDTLS_ERR_SSL_WANT_WRITE);
    }

    if(result != 0){
      throw runtime_error("mbedtls_ssl_handshake failed with code: " +  result);
    }

    cout << "Handshake successful. Connected!" << endl;
  }


  void DtlsClient::_deallocate()
  {
    mbedtls_net_free(&m_serverFd);
    mbedtls_x509_crt_free(&m_cacert);
    mbedtls_ssl_free(&m_ssl);
    mbedtls_ssl_config_free(&m_conf);
    mbedtls_ctr_drbg_free(&m_ctrDrbg);
    mbedtls_entropy_free(&m_entropy);
  }
}