#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include "../lib/libmongoose/mongoose.h"
#include "mem_ops.h"

// return json with compound interest of years
void calc_compound_interest(struct mg_connection *c,double principle, double rate, double time); 
