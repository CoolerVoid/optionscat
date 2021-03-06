#ifndef __VALIDATE_H__
#define __VALIDATE_H__

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include "BSD/strsec.h"
#include "mem_ops.h"

bool validate_bs(char *strike, char *volatility, char *under, char *risk, char *maturity);
bool validate_compound(char *value,char *years,char *percent);

#endif

