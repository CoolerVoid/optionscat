#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include "mem_ops.h"
#include "../lib/libmongoose/mongoose.h"
// colors macros 
#define RED "\033[22;31m"
#define YELLOW "\033[01;33m"
#define CYAN "\033[22;36m"
#define GREEN "\033[22;32m"
#define LAST "\033[0m"

char *file_content(const char * filename); 
char *ReadLines(char * NameFile);
void Write_File(char *file,char *str);
