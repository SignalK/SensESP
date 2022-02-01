#include "hash.h"

#include "mbedtls/md.h"

extern "C" {
#include "crypto/base64.h"
}

/**
 * @brief SHA-1 hash function
 *
 * Calculate a SHA-1 hash of the given payload.
 *
 * @param payload_str
 * @param hash_output A 20-character output array
 * @return String
 */
void Sha1(String payload_str, uint8_t *hash_output) {
  const char *payload = payload_str.c_str();

  const int size = 20;

  mbedtls_md_context_t ctx;
  mbedtls_md_type_t md_type = MBEDTLS_MD_SHA1;

  const size_t payload_length = strlen(payload);

  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
  mbedtls_md_starts(&ctx);
  mbedtls_md_update(&ctx, (const unsigned char *)payload, payload_length);
  mbedtls_md_finish(&ctx, hash_output);
  mbedtls_md_free(&ctx);
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

  size_t output_length;

  Sha1(payload_str, hash_output);

  unsigned char *encoded =
      base64_encode((const unsigned char *)hash_output, 20, &output_length);

  String encoded_str((char *)encoded);

  encoded_str.replace("/", "_");

  free(encoded);

  return encoded_str;
}
