#include "http_authenticator.h"

namespace sensesp {

String get_random_hex_string() {
  char buffer[33];  // buffer to hold 32 Hex Digit + /0
  int i;
  for (i = 0; i < 4; i++) {
    sprintf(buffer + (i * 8), "%08x", esp_random());
  }
  return String(buffer);
}

bool HTTPDigestAuthenticator::authenticate_request(httpd_req_t* req) {
  int result = authenticate_digest(req);

  if (result == 1) {
    return true;
  } else if (result == 0) {
    request_authentication(req, false);
    return 0;
  } else {
    request_authentication(req, true);
    return 0;
  }
}

int HTTPDigestAuthenticator::authenticate_digest(httpd_req_t* req) {
  char auth_header[1024];
  if (httpd_req_get_hdr_value_str(req, "Authorization", auth_header, 1024) !=
      ESP_OK) {
    return 0;
  }

  String auth_header_str(auth_header);
  int space_pos = auth_header_str.indexOf(' ');
  if (space_pos == -1) {
    return 0;
  }

  String auth_type = auth_header_str.substring(0, space_pos);
  if (auth_type != "Digest") {
    return 0;
  }

  String auth_str = auth_header_str.substring(space_pos + 1);

  String username = extract_param("username", auth_str);
  String userhash = extract_param("userhash", auth_str, false);

  if (userhash == "true") {
    String ref_username = MD5(username_ + ":" + realm_);
    if (username != ref_username) {
      return 0;
    }
  } else if (username != username_) {
    // FIXME: Should we disallow unhashed usernames?
    return 0;
  }

  // Extract required parameters from the auth string
  String realm = extract_param("realm", auth_str);
  String nonce = extract_param("nonce", auth_str);
  String uri = extract_param("uri", auth_str);
  String algorithm = extract_param("algorithm", auth_str, false);
  String qop = extract_param("qop", auth_str, false);
  String response = extract_param("response", auth_str);
  String opaque = extract_param("opaque", auth_str);
  String nc = extract_param("nc", auth_str, false);
  String cnonce = extract_param("cnonce", auth_str);

  // check that all required parameters are present
  if (realm == "" || nonce == "" || uri == "" || response == "" ||
      opaque == "" || nc == "" || cnonce == "") {
    return 0;
  }

  // Get the nonce count as an integer
  int count = strtol(nc.c_str(), NULL, 16);

  // Check if the nonce is valid
  int nonce_status = find_nonce(nonce, count);
  if (nonce_status == 0) {
    return 0;
  } else if (nonce_status == -1) {
    return -1;
  }

  // Check that the realm matches
  if (realm != realm_) {
    return 0;
  }

  int method = req->method;
  String method_str;
  switch (method) {
    case HTTP_GET:
      method_str = "GET";
      break;
    case HTTP_POST:
      method_str = "POST";
      break;
    case HTTP_PUT:
      method_str = "PUT";
      break;
    case HTTP_DELETE:
      method_str = "DELETE";
      break;
    default:
      return 0;
  }

  // Calculate the expected response
  String a1 = username_ + ":" + realm_ + ":" + password_;
  String a2 = method_str + ":" + uri;
  String expected_response = MD5(MD5(a1) + ":" + nonce + ":" + nc + ":" +
                                 cnonce + ":" + qop + ":" + MD5(a2));

  return response == expected_response ? 1 : 0;
}

esp_err_t HTTPDigestAuthenticator::request_authentication(httpd_req_t* req,
                                                          bool stale) {
  String server_nonce = create_nonce();
  String opaque = get_random_hex_string();

  httpd_resp_set_status(req, "401 Unauthorized");

  String header = "Digest realm=\"" + realm_ +
                  "\", qop=\"auth\", "
                  "userhash=true, "
                  "algorithm=\"MD5\", nonce=\"" +
                  server_nonce + "\", opaque=\"" + opaque + "\"";

  if (stale) {
    header += ", stale=true";
  }

  httpd_resp_set_hdr(req, "WWW-Authenticate", header.c_str());
  httpd_resp_send(req, "401 Unauthorized", 0);
  return ESP_OK;
}

String HTTPDigestAuthenticator::create_nonce() {
  // The nonce to return is of the form:
  // "timestamp MD5(timestamp:secret)"

  String timestamp_str = String(millis());
  String to_hash = timestamp_str + ":" + secret_;
  String hash = MD5(to_hash);
  String nonce = timestamp_str + " " + hash;

  // The nonce should be base64-encoded
  char encoded[64];
  size_t output_length;
  mbedtls_base64_encode((unsigned char*)encoded, sizeof(encoded),
                        &output_length, (uint8_t*)nonce.c_str(),
                        nonce.length());
  encoded[output_length] = '\0';
  String nonce_str(encoded);
  // Add un-encoded nonce to the list of nonces
  nonces_.push_front({nonce, 1});
  return nonce_str;
}

int HTTPDigestAuthenticator::find_nonce(String nonce, int count) {
  std::list<NonceData>::iterator it;
  String decoded_nonce;
  // Decode the base64 encoding
  size_t output_length;
  char decoded[64];
  mbedtls_base64_decode((unsigned char*)decoded, sizeof(decoded),
                        &output_length, (uint8_t*)nonce.c_str(),
                        nonce.length());
  decoded[output_length] = '\0';
  decoded_nonce = String(decoded);
  for (it = nonces_.begin(); it != nonces_.end(); it++) {
    if (it->nonce == decoded_nonce) {
      // Disable nonce count checking; at least Chrome sends NC values out of
      // order if (it->count > count) {
      //   return 0;
      // }
      it->count++;
      // Check if the nonce is stale
      String timestamp_str = decoded_nonce.substring(0, nonce.indexOf(' '));
      unsigned long timestamp = timestamp_str.toInt();
      if (millis() - timestamp > nonce_max_age_) {
        nonces_.erase(it);
        return -1;
      }
      it->count = count + 1;
      return 1;
    }
    // If the nonce age is 4*nonce_max_age_, remove it from the list
    String timestamp_str = it->nonce.substring(0, it->nonce.indexOf(' '));
    unsigned long timestamp = timestamp_str.toInt();
    if (millis() - timestamp > 4 * nonce_max_age_) {
      it = nonces_.erase(it);
    }
  }
  return 0;
}

String HTTPDigestAuthenticator::extract_param(String param, String auth_str,
                                              bool quoted) {
  String quote = quoted ? "\"" : "";
  int start = auth_str.indexOf(param + "=" + quote);
  if (start == -1) {
    return "";
  }
  start += param.length() + 1 + (quoted ? 1 : 0);
  int end = auth_str.indexOf(quoted ? quote : ",", start);
  if (end == -1) {
    end = auth_str.length();
  }
  return auth_str.substring(start, end);
}

}  // namespace sensesp
