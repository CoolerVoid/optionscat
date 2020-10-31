CC=gcc
CFLAGS=-Wall -Wextra -Wno-unknown-pragmas -O2 
DFLAGS=-DMG_ENABLE_SSL -DMG_DISABLE_PFS lib/libmongoose/mongoose.c lib/BSD/strsec.c lib/frozen/frozen.c 
HARDENING= -mmitigate-rop -fstack-protector-all -pie -fPIE -ftrapv
DIR=src/
DIROUT=bin/
UNAME_S := $(shell uname -s)
#ifeq ($(UNAME_S),Darwin)
LDFLAGS=-lpthread -lcrypto -lssl -lm 
#else
#	LDFLAGS=-Wl,-z,relro,-z,now -lpthread -lm -lcrypto -lssl
#endif

optionscat: $(DIR)optionscat.c 
	@echo "Compile... "
	$(CC) $(CFLAGS) $(DFLAGS) -c $(DIR)*.c 
	$(CC) -o $(DIROUT)optionscat *.o $(LDFLAGS) $(HARDENING)
	$(shell make clean)
	@echo "  "
	@echo " Next step, execute \"bin/optionscat\" to start...  "   
clean:
	rm -f *.o optionscat
