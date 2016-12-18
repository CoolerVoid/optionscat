/*
 * Copyright (c) 2015 Cesanta Software Limited
 * All rights reserved
 */

#include "ktypes.h"

NS_INTERNAL tls_sec_t tls_new_security(void) {
  struct tls_security *sec;

  sec = calloc(1, sizeof(*sec));
  if (NULL == sec) return NULL;

  SHA256_Init(&sec->handshakes_hash);

  return sec;
}

NS_INTERNAL void tls_free_security(tls_sec_t sec) {
  if (sec) {
    const kr_cipher_info *ci = kr_cipher_get_info(sec->cipher_suite);
    RSA_free(sec->svr_key);
    if (sec->server_write_ctx != NULL) ci->free_ctx(sec->server_write_ctx);
    if (sec->client_write_ctx != NULL) ci->free_ctx(sec->client_write_ctx);
    free(sec);
  }
}

NS_INTERNAL void tls_compute_master_secret(tls_sec_t sec,
                                           struct tls_premaster_secret *pre) {
  uint8_t buf[13 + sizeof(sec->cl_rnd) + sizeof(sec->sv_rnd)];

  memcpy(buf, "master secret", 13);
  memcpy(buf + 13, &sec->cl_rnd, sizeof(sec->cl_rnd));
  memcpy(buf + 13 + sizeof(sec->cl_rnd), &sec->sv_rnd, sizeof(sec->sv_rnd));

  prf((uint8_t *) pre, sizeof(*pre), buf, sizeof(buf), sec->master_secret,
      sizeof(sec->master_secret));
#if 0
	printf(" + pre-material\n");
	hex_dump(buf, sizeof(buf), 0);
	printf(" + master secret\n");
	hex_dump(sec->master_secret, sizeof(sec->master_secret), 0);
#endif
}

NS_INTERNAL int tls_check_server_finished(tls_sec_t sec, const uint8_t *vrfy,
                                          size_t vrfy_len) {
  uint8_t buf[15 + SHA256_SIZE];
  uint8_t check[12];
  SHA256_CTX tmp_hash;

  assert(sizeof(check) >= vrfy_len);

  /* don't interfere with running hash */
  memcpy(&tmp_hash, &sec->handshakes_hash, sizeof(tmp_hash));

  memcpy(buf, "server finished", 15);
  SHA256_Final(buf + 15, &tmp_hash);

  prf(sec->master_secret, sizeof(sec->master_secret), buf, sizeof(buf), check,
      vrfy_len);

  return !memcmp(check, vrfy, sizeof(check));
}

NS_INTERNAL int tls_check_client_finished(tls_sec_t sec, const uint8_t *vrfy,
                                          size_t vrfy_len) {
  uint8_t buf[15 + SHA256_SIZE];
  uint8_t check[12];
  SHA256_CTX tmp_hash;

  assert(sizeof(check) >= vrfy_len);

  /* don't interfere with running hash */
  memcpy(&tmp_hash, &sec->handshakes_hash, sizeof(tmp_hash));

  memcpy(buf, "client finished", 15);
  SHA256_Final(buf + 15, &tmp_hash);

  prf(sec->master_secret, sizeof(sec->master_secret), buf, sizeof(buf), check,
      vrfy_len);

  return !memcmp(check, vrfy, vrfy_len);
}

NS_INTERNAL void tls_generate_server_finished(tls_sec_t sec, uint8_t *vrfy,
                                              size_t vrfy_len) {
  uint8_t buf[15 + SHA256_SIZE];
  SHA256_CTX tmp_hash;

  /* don't interfere with running hash */
  memcpy(&tmp_hash, &sec->handshakes_hash, sizeof(tmp_hash));

  memcpy(buf, "server finished", 15);
  SHA256_Final(buf + 15, &tmp_hash);

  prf(sec->master_secret, sizeof(sec->master_secret), buf, sizeof(buf), vrfy,
      vrfy_len);
}

NS_INTERNAL void tls_generate_client_finished(tls_sec_t sec, uint8_t *vrfy,
                                              size_t vrfy_len) {
  uint8_t buf[15 + SHA256_SIZE];
  SHA256_CTX tmp_hash;

  /* don't interfere with running hash */
  memcpy(&tmp_hash, &sec->handshakes_hash, sizeof(tmp_hash));

  memcpy(buf, "client finished", 15);
  SHA256_Final(buf + 15, &tmp_hash);

  prf(sec->master_secret, sizeof(sec->master_secret), buf, sizeof(buf), vrfy,
      vrfy_len);
}

NS_INTERNAL void tls_generate_keys(tls_sec_t sec, int is_server) {
  uint8_t buf[13 + sizeof(sec->cl_rnd) + sizeof(sec->sv_rnd)];
  int mac_len = kr_hmac_len(sec->cipher_suite);
  const kr_cipher_info *ci = kr_cipher_get_info(sec->cipher_suite);

  memcpy(buf, "key expansion", 13);
  memcpy(buf + 13, &sec->sv_rnd, sizeof(sec->sv_rnd));
  memcpy(buf + 13 + sizeof(sec->sv_rnd), &sec->cl_rnd, sizeof(sec->cl_rnd));

  prf(sec->master_secret, sizeof(sec->master_secret), buf, sizeof(buf),
      sec->keys, sizeof(sec->keys));

  sec->client_write_ctx = ci->new_ctx();
  sec->server_write_ctx = ci->new_ctx();
  if (is_server) {
    ci->setup_dec(sec->client_write_ctx, sec->keys + 2 * mac_len);
    ci->setup_enc(sec->server_write_ctx, sec->keys + 2 * mac_len + ci->key_len);
  } else {
    ci->setup_enc(sec->client_write_ctx, sec->keys + 2 * mac_len);
    ci->setup_dec(sec->server_write_ctx, sec->keys + 2 * mac_len + ci->key_len);
  }
}

NS_INTERNAL int tls_tx_push(SSL *ssl, const void *data, size_t len) {
  if (ssl->tx_len + len > ssl->tx_max_len) {
    size_t new_len;
    void *new;

    new_len = ssl->tx_max_len + (len < 512 ? 512 : len);
    new = realloc(ssl->tx_buf, new_len);
    if (NULL == new) {
      /* or block? */
      ssl_err(ssl, SSL_ERROR_SYSCALL);
      return 0;
    }

    ssl->tx_buf = new;
    ssl->tx_max_len = new_len;
  }

  memcpy(ssl->tx_buf + ssl->tx_len, data, len);
  ssl->tx_len += len;

  return 1;
}

NS_INTERNAL int tls_send_enc(SSL *ssl, uint8_t type, const void *buf,
                             size_t len) {
  struct tls_hdr hdr;
  int hdr_offset, enc_offset, enc_len;

  const int mac_len = kr_hmac_len(ssl->cur->cipher_suite);
  const kr_cipher_info *ci = kr_cipher_get_info(ssl->cur->cipher_suite);
  /* Only CBC mode for block ciphers for now, so block cipher -> CBC. */
  const int is_cbc = (ci->block_len > 1);
  const size_t max =
      (1 << 14) - mac_len - (is_cbc ? ci->iv_len + ci->block_len : 0);
  void *cctx =
      ssl->is_server ? ssl->cur->server_write_ctx : ssl->cur->client_write_ctx;
  uint8_t pad_len = 0;
  uint8_t iv[MAX_IV_SIZE];

  if (len > max) len = max;

  /* Header */
  if (is_cbc) {
    pad_len = ci->block_len - ((len + mac_len + 1) % ci->block_len);
  }

  hdr.type = type;
  hdr.vers = htobe16(TLS_1_2_PROTO);
  hdr.len = 0; /* will fill in at the end. */

  hdr_offset = ssl->tx_len;
  if (!tls_tx_push(ssl, &hdr, sizeof(hdr))) return 0;

  /* Explicit IV for CBC mode. */
  if (is_cbc) {
    /* Seed with system PRNG and mix in our state. */
    kr_get_random(iv, ci->iv_len);
    prf(iv, ci->iv_len, (uint8_t *) ssl, sizeof(*ssl), iv, ci->iv_len);
    if (!tls_tx_push(ssl, iv, ci->iv_len)) return 0;
    hdr.len += ci->iv_len;
  }

  enc_offset = ssl->tx_len;
  if (!tls_tx_push(ssl, buf, len)) return 0;
  enc_len = len;

  { /* MAC */
    struct tls_hmac_hdr phdr;
    const uint8_t *msgs[2];
    size_t msgl[2];
    uint8_t digest[MAX_DIGEST_SIZE];
    if (ssl->is_server) {
      phdr.seq = htobe64(ssl->cur->server_write_seq);
    } else {
      phdr.seq = htobe64(ssl->cur->client_write_seq);
    }
    phdr.type = hdr.type;
    phdr.vers = hdr.vers;
    phdr.len = htobe16(len);

    msgs[0] = (uint8_t *) &phdr;
    msgl[0] = sizeof(phdr);
    msgs[1] = buf;
    msgl[1] = len;
    kr_ssl_hmac(ssl, ssl->is_server ? KR_SERVER_MAC : KR_CLIENT_MAC, 2, msgs,
                msgl, digest);

    if (!tls_tx_push(ssl, digest, mac_len)) return 0;
    enc_len += mac_len;
  }

  /* Padding for CBC mode. */
  if (is_cbc) {
    int i;
    dprintf(("len %d block_len %d pad_len %d\n", (int) len, (int) ci->block_len,
             (int) pad_len));
    for (i = 0; i < pad_len; i++) {
      if (!tls_tx_push(ssl, &pad_len, 1)) return 0;
    }
    if (!tls_tx_push(ssl, &pad_len, 1)) return 0;
    enc_len += pad_len + 1;
  }

  /* Encryption. */
  if (is_cbc) {
    kr_cbc_encrypt(ci, cctx, ssl->tx_buf + enc_offset, enc_len, iv,
                   ssl->tx_buf + enc_offset);
  } else {
    ci->encrypt(cctx, ssl->tx_buf + enc_offset, enc_len,
                ssl->tx_buf + enc_offset);
  }

  hdr.len = htobe16(hdr.len + enc_len);
  memcpy(ssl->tx_buf + hdr_offset, &hdr, sizeof(hdr));

  if (ssl->is_server) {
    ssl->cur->server_write_seq++;
  } else {
    ssl->cur->client_write_seq++;
  }

  return len;
}

NS_INTERNAL int tls_send(SSL *ssl, uint8_t type, const void *buf, size_t len) {
  if (type == TLS_HANDSHAKE &&
      ((const uint8_t *) buf)[0] != HANDSHAKE_HELLO_REQ) {
    tls_add_handshake_to_hash(ssl, buf, len);
  }
  if (ssl->tx_enc) {
    return tls_send_enc(ssl, type, buf, len);
  } else {
    struct tls_hdr hdr;
    size_t max = (1 << 14);
    if (len > max) len = max;
    hdr.type = type;
    hdr.vers = htobe16(TLS_1_2_PROTO);
    hdr.len = htobe16(len);

    if (!tls_tx_push(ssl, &hdr, sizeof(hdr))) return 0;
    if (!tls_tx_push(ssl, buf, len)) return 0;

    return len;
  }
}

NS_INTERNAL void tls_add_handshake_to_hash(SSL *ssl, const void *data,
                                           size_t len) {
  if (ssl->cur) SHA256_Update(&ssl->cur->handshakes_hash, data, len);
  if (ssl->nxt) SHA256_Update(&ssl->nxt->handshakes_hash, data, len);
}

NS_INTERNAL int tls_send_certs(SSL *ssl, const PEM *certs) {
  unsigned int i;
  struct tls_hdr hdr;
  struct tls_cert cert;
  struct tls_cert_hdr chdr;

  if (certs == NULL) return 0;

  dprintf(("sending %d certs\n", (int) certs->num_obj));

  hdr.type = TLS_HANDSHAKE;
  hdr.vers = htobe16(TLS_1_2_PROTO);
  hdr.len =
      htobe16(sizeof(cert) + sizeof(chdr) * certs->num_obj + certs->tot_len);

  if (!tls_tx_push(ssl, &hdr, sizeof(hdr))) return 0;

  cert.type = HANDSHAKE_CERTIFICATE;
  cert.len_hi = 0;
  cert.len =
      htobe16(sizeof(chdr) + sizeof(chdr) * certs->num_obj + certs->tot_len);
  cert.certs_len_hi = 0;
  cert.certs_len = htobe16(sizeof(chdr) * certs->num_obj + certs->tot_len);

  if (!tls_tx_push(ssl, &cert, sizeof(cert))) return 0;

  tls_add_handshake_to_hash(ssl, &cert, sizeof(cert));

  for (i = 0; i < certs->num_obj; i++) {
    DER *d = &certs->obj[i];

    chdr.cert_len_hi = 0;
    chdr.cert_len = htobe16(d->der_len);

    if (!tls_tx_push(ssl, &chdr, sizeof(chdr))) return 0;
    tls_add_handshake_to_hash(ssl, &chdr, sizeof(chdr));
    if (!tls_tx_push(ssl, d->der, d->der_len)) return 0;
    tls_add_handshake_to_hash(ssl, d->der, d->der_len);
  }

  return 1;
}

NS_INTERNAL ssize_t tls_write(SSL *ssl, const uint8_t *buf, size_t sz) {
  /* FIXME: break up in to max-sized packets */
  int res = tls_send(ssl, TLS_APP_DATA, buf, sz);
  return res == 0 ? -1 : res;
}

NS_INTERNAL int tls_alert(SSL *ssl, uint8_t level, uint8_t desc) {
  struct tls_alert alert;
  if (ssl->fatal) return 1;
  if (level == ALERT_LEVEL_FATAL) {
    ssl->fatal = 1;
    ssl->tx_len = 0; /* Flush send buffer, leave an alert. */
  }
  alert.level = level;
  alert.desc = desc;
  return tls_send(ssl, TLS_ALERT, &alert, sizeof(alert));
}

NS_INTERNAL int tls_close_notify(SSL *ssl) {
  return tls_alert(ssl, ALERT_LEVEL_WARNING, ALERT_CLOSE_NOTIFY);
}
