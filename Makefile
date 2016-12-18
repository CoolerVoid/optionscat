CC=gcc
CFLAGS=-Wall -Wextra -Wno-unknown-pragmas -O2 
DFLAGS=-DMG_ENABLE_SSL -DMG_DISABLE_PFS lib/krypton/krypton.c lib/libmongoose/mongoose.c lib/BSD/strsec.c lib/frozen/frozen.c -llib/krypton
DIR=src/
DIROUT=bin/
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	LDFLAGS=-lpthread 
else
	LDFLAGS=-Wl,-z,relro,-z,now -lpthread -lm 
endif

optionscat: $(DIR)optionscat.c 
	@echo "Compile... "
	$(CC) $(CFLAGS) $(DFLAGS) -c $(DIR)*.c 
	$(CC) -o $(DIROUT)optionscat *.o $(LDFLAGS)
	$(shell make clean)
	@echo "  "
	@echo " Next step, execute \"bin/optionscat\" to start...  "   
clean:
	rm -f *.o optionscat
