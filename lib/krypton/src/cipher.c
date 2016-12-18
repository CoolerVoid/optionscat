/*
 * Copyright (c) 2015 Cesanta Software Limited
 * All rights reserved
 */

#include "ktypes.h"

#if KR_ALLOW_NULL_CIPHERS

NS_INTERNAL void *kr_null_new_ctx() {
  return NULL;
}

NS_INTERNAL void kr_null_setup(void *ctxv, const uint8_t *key) {
  (void) ctxv;
  (void) key;
}

NS_INTERNAL void kr_null_crypt(void *ctxv, const uint8_t *msg, int len,
                               uint8_t *out) {
  (void) ctxv;
  memmove(out, msg, len);
}

NS_INTERNAL void kr_null_free_ctx(void *ctxv) {
  (void) ctxv;
}

static const kr_cipher_info null_cs_info = {
    1, 0, 0, kr_null_new_ctx, kr_null_setup, kr_null_setup, kr_null_crypt,
    kr_null_crypt, kr_null_free_ctx};
#endif

NS_INTERNAL const kr_cipher_info *kr_cipher_get_info(kr_cs_id cs) {
  switch (cs) {
#if KR_ALLOW_NULL_CIPHERS
    case TLS_RSA_WITH_NULL_MD5:
      return &null_cs_info;
#endif
    case TLS_RSA_WITH_RC4_128_MD5:
    case TLS_RSA_WITH_RC4_128_SHA:
      return kr_rc4_cs_info();
    case TLS_RSA_WITH_AES_128_CBC_SHA:
    case TLS_RSA_WITH_AES_128_CBC_SHA256:
      return kr_aes128_cs_info();
  }
  return NULL;
}

NS_INTERNAL void kr_cbc_encrypt(const kr_cipher_info *ci, void *cctx,
                                const uint8_t *in, int len, const uint8_t *iv,
                                uint8_t *out) {
  int i;
  uint32_t d32[4], xor[4];

  assert(ci->iv_len == 16);
  assert(ci->block_len == 16);

  memcpy (xor, iv, ci->iv_len);

  for (len -= ci->block_len; len >= 0; len -= ci->block_len) {
    memcpy(d32, in, ci->block_len);
    in += ci->block_len;

    for (i = 0; i < 4; i++) d32[i] ^= xor[i];

    ci->encrypt(cctx, (const uint8_t *) d32, ci->block_len, (uint8_t *) d32);

    memcpy (xor, d32, ci->block_len);
    memcpy(out, d32, ci->block_len);
    out += ci->block_len;
  }
}

NS_INTERNAL void kr_cbc_decrypt(const kr_cipher_info *ci, void *cctx,
                                const uint8_t *in, int len, const uint8_t *iv,
                                uint8_t *out) {
  int i;
  uint32_t d32[4], xor[4];

  assert(ci->iv_len == 16);
  assert(ci->block_len == 16);
  memcpy (xor, iv, ci->iv_len);

  for (len -= ci->block_len; len >= 0; len -= ci->block_len) {
    memcpy(d32, in, ci->block_len);

    ci->decrypt(cctx, (const uint8_t *) d32, ci->block_len, (uint8_t *) d32);

    for (i = 0; i < 4; i++) d32[i] ^= xor[i];

    memcpy (xor, in, ci->block_len);
    in += ci->block_len;

    memcpy(out, d32, ci->block_len);
    out += ci->block_len;
  }
}
