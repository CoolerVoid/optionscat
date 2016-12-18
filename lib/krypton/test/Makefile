# Copyright (c) 2015 Cesanta Software Limited
# All rights reserved

.PHONY: all tests run-tests clean

CFLAGS := -O0 -W -Wall -Werror -g -I../.. \
          -DKRYPTON_DEBUG -DDEBUG_VERIFY -DKR_MODULE_LINES \
          $(CFLAGS_EXTRA)
CFLAGS_KRYPTON := -I..
OPENSSL_FLAGS := -lssl -lcrypto

ifdef ASAN
CC = clang-3.6
CFLAGS += -fsanitize=address -fcolor-diagnostics
endif

.PHONY: all amalgam clean tests run-tests

all: tests

amalgam:
	make -C .. krypton.c

tests: amalgam unit-test sv-test-openssl cl-test-openssl sv-test-krypton cl-test-krypton

unit-test: unit-test.c ../krypton.c ../../common/test_util.c ../../common/cs_time.c
	$(CC) $(CFLAGS) $(CFLAGS_KRYPTON) -DNS_INTERNAL= -I../src -o unit-test $^

sv-test-openssl: sv-test.c
	$(CC) $(CFLAGS) -o sv-test-openssl $^ $(OPENSSL_FLAGS)

cl-test-openssl: cl-test.c
	$(CC) $(CFLAGS) -o cl-test-openssl $^ $(OPENSSL_FLAGS)

%-test-krypton: CFLAGS += -DUSE_KRYPTON=1 -I.

sv-test-krypton: sv-test.c ../krypton.c
	$(CC) $(CFLAGS) $(CFLAGS_KRYPTON) -o sv-test-krypton $^

cl-test-krypton: cl-test.c ../krypton.c
	$(CC) $(CFLAGS) $(CFLAGS_KRYPTON) -o cl-test-krypton $^

win-test: ../krypton.c
ifndef VC6_DIR
	$(error Please set VC6_DIR)
endif
	Include=$(VC6_DIR)/include Lib=$(VC6_DIR)/lib \
	wine $(VC6_DIR)/bin/cl win-test.c ../krypton.c

run-tests: tests
	./unit-test
	./sv-test-openssl & ./cl-test-krypton -a ca.pem -n
	./sv-test-openssl -C AES128-SHA & ./cl-test-krypton -a ca.pem -n
	./sv-test-openssl -C RC4-SHA & ./cl-test-krypton -a ca.pem -n
	./sv-test-openssl -a ca.pem & ./cl-test-krypton -a ca.pem -n -c client.pem -k client.key
	./sv-test-krypton & ./cl-test-openssl -a ca.pem
	./sv-test-krypton & ./cl-test-krypton -a ca.pem -n

clean:
	rm -f cl-test-* sv-test-* unit-test
