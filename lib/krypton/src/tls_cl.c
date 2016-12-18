/*
 * Copyright (c) 2015 Cesanta Software Limited
 * All rights reserved
 */

#include "../openssl/ssl.h"
#include "ktypes.h"
#include "tls.h"
#include "tlsproto.h"

#include <time.h>

NS_INTERNAL int tls_cl_hello(SSL *ssl) {
  int i = 0;
  struct tls_cl_hello hello;

  /* hello */
  hello.type = HANDSHAKE_CLIENT_HELLO;
  hello.len_hi = 0;
  hello.len = htobe16(sizeof(hello) - 4);
  hello.version = htobe16(0x0303);
  hello.random.time = htobe32(time(NULL));
  if (!kr_get_random(hello.random.opaque, sizeof(hello.random.opaque))) {
    ssl_err(ssl, SSL_ERROR_SYSCALL);
    return 0;
  }
  hello.sess_id_len = 0;
#if KR_ALLOW_NULL_CIPHERS
  /* if we allow them, it's for testing reasons, so NULL comes first */
  hello.cipher_suite[i++] = htobe16(TLS_RSA_WITH_NULL_MD5);
  hello.cipher_suite[i++] = htobe16(TLS_RSA_WITH_AES_128_CBC_SHA256);
  hello.cipher_suite[i++] = htobe16(TLS_RSA_WITH_AES_128_CBC_SHA);
  hello.cipher_suite[i++] = htobe16(TLS_RSA_WITH_RC4_128_SHA);
  hello.cipher_suite[i++] = htobe16(TLS_RSA_WITH_RC4_128_MD5);
  hello.cipher_suite[i++] = htobe16(TLS_EMPTY_RENEGOTIATION_INFO_SCSV);
#else
  hello.cipher_suite[i++] = htobe16(TLS_RSA_WITH_AES_128_CBC_SHA256);
  hello.cipher_suite[i++] = htobe16(TLS_RSA_WITH_AES_128_CBC_SHA);
  hello.cipher_suite[i++] = htobe16(TLS_RSA_WITH_RC4_128_SHA);
  hello.cipher_suite[i++] = htobe16(TLS_RSA_WITH_RC4_128_MD5);
  hello.cipher_suite[i++] = htobe16(TLS_EMPTY_RENEGOTIATION_INFO_SCSV);
#endif
  hello.cipher_suites_len = htobe16(i * 2);
  hello.num_compressors = 1;
  hello.compressor[0] = COMPRESSOR_NULL;
  hello.ext_len = htobe16(sizeof(hello.ext_reneg));

  hello.ext_reneg.type = htobe16(EXT_RENEG_INFO);
  hello.ext_reneg.len = htobe16(1);
  hello.ext_reneg.ri_len = 0;

  if (!tls_send(ssl, TLS_HANDSHAKE, &hello, sizeof(hello))) return 0;

  /* store the random we generated */
  memcpy(&ssl->nxt->cl_rnd, &hello.random, sizeof(ssl->nxt->cl_rnd));

  return 1;
}

static void set16(unsigned char *p, uint16_t v) {
  p[0] = (v >> 8) & 0xff;
  p[1] = v & 0xff;
}

NS_INTERNAL int tls_cl_finish(SSL *ssl) {
  struct tls_change_cipher_spec cipher;
  struct tls_finished finished;
  size_t buf_len = 6 + RSA_block_size(ssl->nxt->svr_key);
  unsigned char buf[6 + 512];
  struct tls_premaster_secret in;

  assert(buf_len < sizeof(buf)); /* Fix this */

  in.version = htobe16(0x0303);
  if (!kr_get_random(in.opaque, sizeof(in.opaque))) {
    ssl_err(ssl, SSL_ERROR_SYSCALL);
    return 0;
  }
  tls_compute_master_secret(ssl->nxt, &in);
  tls_generate_keys(ssl->nxt, ssl->is_server);
  dprintf((" + master secret computed\n"));

  if (RSA_encrypt(ssl->nxt->svr_key, (uint8_t *) &in, sizeof(in), buf + 6, 0) <=
      1) {
    dprintf(("RSA encrypt failed\n"));
    ssl_err(ssl, SSL_ERROR_SSL);
    return 0;
  }

  buf[0] = HANDSHAKE_CLIENT_KEY_EXCH;
  buf[1] = 0;
  set16(buf + 2, buf_len - 4);
  set16(buf + 4, buf_len - 6);
  if (!tls_send(ssl, TLS_HANDSHAKE, buf, buf_len)) return 0;

  /* cert verify, if required */
  if (ssl->cert_requested && ssl->ctx->pem_cert != NULL) {
    SHA256_CTX tmp_hash;
    uint8_t tmp_digest[SHA256_SIZE + 19];
    uint8_t *p = buf;
    *p++ = HANDSHAKE_CERTIFICATE_VRFY;
    *p++ = 0;
    buf_len = 2 + 2 + RSA_block_size(ssl->ctx->rsa_privkey);
    set16(p, buf_len);
    p += 2;
    *p++ = TLS_HASH_SHA256;
    *p++ = TLS_SIG_RSA;
    memcpy(&tmp_hash, &ssl->nxt->handshakes_hash, sizeof(tmp_hash));
    /*
     * This is the RSASSA-PKCS1-v1_5 header for a SHA256 digest,
     * and translates from ASN.1-speak as follows:\
     * SEQUENCE (2 elem)
     *   SEQUENCE (2 elem)
     *     OBJECT IDENTIFIER  2.16.840.1.101.3.4.2.1  (id-sha256)
     *     NULL
     *   OCTET STRING (32 byte)
     */
    memcpy(tmp_digest,
           "\x30\x31\x30\x0D\x06\x09\x60\x86\x48\x01\x65\x03\x04\x02\x01\x05"
           "\x00\x04\x20",
           19);
    SHA256_Final(tmp_digest + 19, &tmp_hash);
    set16(p, RSA_block_size(ssl->ctx->rsa_privkey));
    p += 2;
    if (RSA_encrypt(ssl->ctx->rsa_privkey, tmp_digest, sizeof(tmp_digest), p,
                    1 /* is_signing */) !=
        RSA_block_size(ssl->ctx->rsa_privkey)) {
      dprintf(("RSA sign failed\n"));
      ssl_err(ssl, SSL_ERROR_SSL);
      return 0;
    }
    p += RSA_block_size(ssl->ctx->rsa_privkey);
    if (!tls_send(ssl, TLS_HANDSHAKE, buf, p - buf)) return 0;
  }

  /* change cipher spec */
  cipher.one = 1;
  if (!tls_send(ssl, TLS_CHANGE_CIPHER_SPEC, &cipher, sizeof(cipher))) return 0;

  if (ssl->cur) {
    tls_free_security(ssl->cur);
  }
  ssl->cur = ssl->nxt;
  ssl->nxt = NULL;
  ssl->tx_enc = 1;

  /* finished */
  finished.type = HANDSHAKE_FINISHED;
  finished.len_hi = 0;
  finished.len = htobe16(sizeof(finished.vrfy));
  memset(finished.vrfy, 0, sizeof(finished.vrfy));
  tls_generate_client_finished(ssl->cur, finished.vrfy, sizeof(finished.vrfy));

  if (!tls_send(ssl, TLS_HANDSHAKE, &finished, sizeof(finished))) return 0;

  return 1;
}
