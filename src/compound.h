#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "../lib/libmongoose/mongoose.h"

// return json with compound interest of years
void calc_compound_interest(struct mg_connection *c,double principle, double rate, double time); 
