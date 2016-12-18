#include <stdio.h>
#include <math.h>
#include <fenv.h>
#include <errno.h>
#include <stdlib.h>
#include "../lib/libmongoose/mongoose.h"
#include "mem_ops.h"
#define _USE_MATH_DEFINES


#define PI 3.14159265358979323846

#pragma STDC FENV_ACCESS ON


// NOTE: this way is for European vanilla options

// Standard normal probability density function
double norm_pdf(const double x); 
/*
 An approximation to the cumulative distribution function
 for the standard normal distribution
*/
double norm_cdf(const double x); 
/*
 This calculates d_j, for j in {1,2}. This term appears in the closed
 form solution for the European call or put price
*/
double d_j(const int j, const double S, const double K, const double r, const double v, const double T);

/* 
	based on
 underlying S, strike K, risk-free rate r, volatility of
 underlying sigma and time to maturity T
*/
double call_price(const double S, const double K, const double r, const double v, const double T);
double call_delta(const double S, const double K, const double r, const double v, const double T); 
double call_gamma(const double S, const double K, const double r, const double v, const double T);
double call_vega(const double S, const double K, const double r, const double v, const double T); 
double call_theta(const double S, const double K, const double r, const double v, const double T); 
double call_rho(const double S, const double K, const double r, const double v, const double T); 
/*
 based on
 underlying S, strike K, risk-free rate r, volatility of
underlying sigma and time to maturity T
*/
double put_price(const double S, const double K, const double r, const double v, const double T); 
double put_delta(const double S, const double K, const double r, const double v, const double T); 
double put_gamma(const double S, const double K, const double r, const double v, const double T); 
double put_vega(const double S, const double K, const double r, const double v, const double T); 
double put_theta(const double S, const double K, const double r, const double v, const double T);
double put_rho(const double S, const double K, const double r, const double v, const double T); 

/* Note is for European Vanilla options
S= Option price
K= Strike price
r= Risk-free rate (5%)
v= Volatility of the underlying (20%)
T= One year until expiry
*/
void create_greek_table(struct mg_connection *c,char *name,char *type, double S, double K, double r, double v, double T); 


//TODO add Asian, american options here...
