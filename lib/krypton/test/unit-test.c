/*
 * Copyright (c) 2014-2016 Cesanta Software Limited
 * All rights reserved
 */

#include <stdio.h>

#include "krypton.h"
#include "ktypes.h"
#include "pem.h"

#include "../../common/test_util.h"

static struct ro_vec s_vec(const char *s) {
  struct ro_vec v;
  v.ptr = (const uint8_t *) s;
  v.len = strlen(s);
  return v;
}

static const char *test_match_domain_name(void) {
  ASSERT_EQ(kr_match_domain_name(s_vec(""), s_vec("")), 1);
  ASSERT_EQ(kr_match_domain_name(s_vec("a"), s_vec("a")), 1);
  ASSERT_EQ(kr_match_domain_name(s_vec("A"), s_vec("a")), 1);
  ASSERT_EQ(kr_match_domain_name(s_vec("a"), s_vec("A")), 1);
  ASSERT_EQ(kr_match_domain_name(s_vec("*"), s_vec("a")), 1);
  ASSERT_EQ(kr_match_domain_name(s_vec("*"), s_vec("abc")), 1);
  ASSERT_EQ(kr_match_domain_name(s_vec("a.b"), s_vec("a.b")), 1);
  ASSERT_EQ(kr_match_domain_name(s_vec("a.b"), s_vec("A.b")), 1);
  ASSERT_EQ(kr_match_domain_name(s_vec("*.b"), s_vec("a.b")), 1);
  ASSERT_EQ(kr_match_domain_name(s_vec("*.d"), s_vec("a.bc.d")), 1);

  ASSERT_EQ(kr_match_domain_name(s_vec("a"), s_vec("b")), 0);
  /* No partial glob matches. */
  ASSERT_EQ(kr_match_domain_name(s_vec("b*.d"), s_vec("bc.d")), 0);
  ASSERT_EQ(kr_match_domain_name(s_vec("aa.b"), s_vec("a.b")), 0);
  ASSERT_EQ(kr_match_domain_name(s_vec("a.a.b"), s_vec("a.b")), 0);
  ASSERT_EQ(kr_match_domain_name(s_vec(""), s_vec("a")), 0);
  ASSERT_EQ(kr_match_domain_name(s_vec("b"), s_vec("a")), 0);
  return NULL;
}

static const char *test_verify_name(void) {
  PEM *pem;
  X509 *cert;
  pem = pem_load_types("server.pem", PEM_SIG_CERT);
  ASSERT(pem != NULL);
  ASSERT_EQ(pem->num_obj, 1);
  cert = X509_new(pem->obj->der, pem->obj->der_len);
  ASSERT(cert != NULL);
  ASSERT_EQ(X509_verify_name(cert, "do.not.use.me"), 1);    /* matches CN */
  ASSERT_EQ(X509_verify_name(cert, "never.use.me"), 1);     /* alt name 1 */
  ASSERT_EQ(X509_verify_name(cert, "except.for.tests"), 1); /* alt name 2 */
  ASSERT_EQ(X509_verify_name(cert, "example.com"), 0);
  ASSERT_EQ(X509_verify_name(cert, "xxx"), 0);
  ASSERT_EQ(X509_verify_name(cert, ""), 0);
  X509_free(cert);
  pem_free(pem);
  return NULL;
}

static const char *run_tests(const char *filter, double *total_elapsed) {
  RUN_TEST(test_match_domain_name);
  RUN_TEST(test_verify_name);
  return NULL;
}

int main(int argc, char *argv[]) {
  const char *fail_msg;
  const char *filter = argc > 1 ? argv[1] : "";
  double total_elapsed = 0.0;

  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);

  fail_msg = run_tests(filter, &total_elapsed);
  printf("%s, run %d in %.3lfs\n", fail_msg ? "FAIL" : "PASS", num_tests,
         total_elapsed);
  return fail_msg == NULL ? 0 : 1;
}
