#include "hash.h"

#include "mbedtls/md.h"
#include "mbedtls/md5.h"
#include "mbedtls/base64.h"

using namespace sensesp;

/**
 * @brief SHA-1 hash function
 *
 * Calculate a SHA-1 hash of the given payload.
 *
 * @param payload_str
 * @param hash_output A 20-character binary output array
 */
void Sha1(String payload_str, uint8_t *hash_output) {
  const char *payload = payload_str.c_str();

  const int size = 20;

  mbedtls_md_context_t ctx;
  mbedtls_md_type_t md_type = MBEDTLS_MD_SHA1;

  const size_t payload_length = payload_str.length();

  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
  mbedtls_md_starts(&ctx);
  mbedtls_md_update(&ctx, (const unsigned char *)payload, payload_length);
  mbedtls_md_finish(&ctx, hash_output);
  mbedtls_md_free(&ctx);
}

/**
 * @brief MD5 hash function
 *
 * Calculate an MD5 hash of the given payload.
 *
 * @param payload_str
 */
String MD5(String payload_str){
  const char *payload = payload_str.c_str();
  char output[33] = {0};

  const size_t payload_length = payload_str.length();

  mbedtls_md5_context _ctx;
  uint8_t i;
  uint8_t _buf[16] = {0};
  mbedtls_md5_init(&_ctx);
  mbedtls_md5_starts_ret(&_ctx);
  mbedtls_md5_update_ret(&_ctx, (const uint8_t *)payload, payload_length);
  mbedtls_md5_finish_ret(&_ctx, _buf);
  mbedtls_md5_free(&_ctx);
  for(i = 0; i < 16; i++) {
    sprintf(output + (i * 2), "%02x", _buf[i]);
  }
  return String(output);
}

/**
 * @brief A base64-encoded SHA-1 hash function
 *
 * For filesystem compatibility, the output string is further scrubbed
 * to replace "/" with "_".
 *
 * @param payload_str
 * @return String
 */
String Base64Sha1(String payload_str) {
  uint8_t hash_output[20];

  uint8_t encoded[32];

  size_t output_length;

  Sha1(payload_str, hash_output);

  int retval = mbedtls_base64_encode(encoded, sizeof(encoded), &output_length, hash_output, 20);

  if (retval != 0) {
    debugE("Base64 encoding failed");
    return "";
  }

  String encoded_str((char *)encoded);

  encoded_str.replace("/", "_");

  return encoded_str;
}
