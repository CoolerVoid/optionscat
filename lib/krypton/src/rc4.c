/*
 * Copyright (c) 2007, Cameron Rich
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * * Neither the name of the axTLS project nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef KR_EXT_RC4

/**
 * An implementation of the RC4/ARC4 algorithm.
 * Originally written by Christophe Devine.
 */

#include "ktypes.h"

typedef struct {
  uint8_t x, y;
  uint8_t m[256];
} kr_rc4_ctx;

NS_INTERNAL void *kr_rc4_new_ctx() {
  return calloc(1, sizeof(kr_rc4_ctx));
}

NS_INTERNAL void kr_rc4_setup(void *ctxv, const uint8_t *key) {
  kr_rc4_ctx *ctx = (kr_rc4_ctx *) ctxv;
  int i, j = 0, k = 0, a;
  uint8_t *m;

  ctx->x = 0;
  ctx->y = 0;
  m = ctx->m;

  for (i = 0; i < 256; i++) m[i] = i;

  for (i = 0; i < 256; i++) {
    a = m[i];
    j = (uint8_t)(j + a + key[k]);
    m[i] = m[j];
    m[j] = a;

    if (++k >= RC4_KEY_SIZE) k = 0;
  }
}

/**
 * Perform the encrypt/decrypt operation (can use it for either since
 * this is a stream cipher).
 */
NS_INTERNAL void kr_rc4_crypt(void *ctxv, const uint8_t *msg, int len,
                              uint8_t *out) {
  kr_rc4_ctx *ctx = (kr_rc4_ctx *) ctxv;
  int i;
  uint8_t *m, x, y, a, b;

  /* NOTE: *msg and *out must be the same pointer (performance tweak) */
  assert(msg == out);
  (void) msg;
  x = ctx->x;
  y = ctx->y;
  m = ctx->m;

  for (i = 0; i < len; i++) {
    a = m[++x];
    y += a;
    m[x] = b = m[y];
    m[y] = a;
    out[i] ^= m[(uint8_t)(a + b)];
  }

  ctx->x = x;
  ctx->y = y;
}

NS_INTERNAL void kr_rc4_free_ctx(void *ctxv) {
  free(ctxv);
}

const kr_cipher_info *kr_rc4_cs_info() {
  static const kr_cipher_info rc4_cs_info = {
      1, RC4_KEY_SIZE, 0, kr_rc4_new_ctx, kr_rc4_setup, kr_rc4_setup,
      kr_rc4_crypt, kr_rc4_crypt, kr_rc4_free_ctx};
  return &rc4_cs_info;
}
#endif /* !KR_EXT_RC4 */
