#ifndef SENSESP_SRC_SENSESP_NET_HTTP_AUTHENTICATOR_H_
#define SENSESP_SRC_SENSESP_NET_HTTP_AUTHENTICATOR_H_

#include <esp_http_server.h>
#include <mbedtls/base64.h>

#include <list>

#include "sensesp.h"
#include "sensesp/system/hash.h"

namespace sensesp {

String get_random_hex_string();

/**
 * @brief HTTP Authenticator base class.
 *
 */
class HTTPAuthenticator {
 public:
  HTTPAuthenticator() {}
  /**
   * @brief Authenticate an incoming request.
   *
   * This method should be called at the beginning of each request handler
   * to ensure that the request is authenticated.
   *
   * @param req
   * @return true Authentication successful.
   * @return false Authentication failed.
   */
  virtual bool authenticate_request(httpd_req_t* req) = 0;
};

struct NonceData {
  String nonce;
  int count;
};

/**
 * @brief HTTP Digest Authenticator class.
 *
 */
class HTTPDigestAuthenticator : public HTTPAuthenticator {
 public:
  HTTPDigestAuthenticator(String username, String password, String realm,
                          unsigned long nonce_max_age = 900000)
      : username_(username),
        password_(password),
        realm_(realm),
        nonce_max_age_(nonce_max_age),
        HTTPAuthenticator() {
    secret_ = get_random_hex_string();
  }

  virtual bool authenticate_request(httpd_req_t* req) override;

  int authenticate_digest(httpd_req_t* req);

 protected:
  esp_err_t request_authentication(httpd_req_t* req, bool stale = false);

  String username_;
  String password_;
  String realm_;
  unsigned long nonce_max_age_;
  String secret_;  ///< Secret used to generate the nonce. Gets reset every
                   ///< restart.
  std::list<NonceData> nonces_;

  String create_nonce();

  /**
   * @brief Find a nonce in the list of nonces.
   *
   * @param nonce Nonce to find
   * @param count Count of the nonce
   * @param max_age Maximum age of the nonce in milliseconds
   * @return 1 Nonce found
   * @return 0 Nonce not found
   * @return -1 Nonce found but stale
   */
  int find_nonce(String nonce, int count);

  String extract_param(String param, String auth_str, bool quoted = true);
};

}  // namespace sensesp

#endif  // SENSESP_SRC_SENSESP_NET_HTTP_AUTHENTICATOR_H_
