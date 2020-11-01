// NOTE: this way is for European vanilla options
#include "greeks.h"
#include "libmongoose/mongoose.h"
#include "html_entities.h"
#include "BSD/strsec.h"

// normal probability density function https://en.wikipedia.org/wiki/Probability_density_function
double norm_pdf(const double x) 
{
	double ret=(1.0/(pow(2*PI,0.5)))*exp(-0.5*x*x);
	int fpeRaised=0;

	fpeRaised=fetestexcept(FE_INVALID | FE_DIVBYZERO |FE_OVERFLOW | FE_UNDERFLOW | FE_INEXACT);

  	if(fpeRaised!=32 && fpeRaised!=0)
	{
		DEBUG("error here!");
   		perror("Log: \n");
		ret = 0;
	}

  	fpeRaised=feclearexcept(FE_ALL_EXCEPT);

	return ret;
}

/*
 cumulative distribution function
 https://en.wikipedia.org/wiki/Cumulative_distribution_function
*/
double norm_cdf(const double x) 
{
	double k = 1.0/(1.0 + 0.2316419*x);
	double k_sum = k*(0.319381530 + k*(-0.356563782 + k*(1.781477937 + k*(-1.821255978 + 1.330274429*k))));
	double ret=0;

  	if (x >= 0.0) 
    		ret = (1.0 - (1.0/(pow(2*PI,0.5)))*exp(-0.5*x*x) * k_sum);
  	else 
    		ret = 1.0 - norm_cdf(-x);
  
	int fpeRaised=0; 

	fpeRaised=fetestexcept(FE_INVALID | FE_DIVBYZERO |FE_OVERFLOW | FE_UNDERFLOW | FE_INEXACT);

  	if(fpeRaised!=32 && fpeRaised!=0)
	{
		DEBUG("error here!");
   		perror("Log: \n");
		ret = 0;
	}

  	fpeRaised=feclearexcept(FE_ALL_EXCEPT);

	return ret;
}

/*
 This calculates d_j, for j in {1,2}. This term appears in the closed
 form solution for the European call or put price
*/
double d_j(const int j, const double S, const double K, const double r, const double v, const double T) 
{
	double ret = (log(S/K) + (r + (pow(-1,j-1))*0.5*v*v)*T)/(v*(pow(T,0.5)));

	int fpeRaised=0;

	fpeRaised=fetestexcept(FE_INVALID | FE_DIVBYZERO |FE_OVERFLOW | FE_UNDERFLOW | FE_INEXACT);

  	if(fpeRaised!=32 && fpeRaised!=0)
	{
		DEBUG("error here!");
   		perror("Log: \n");
		ret = 0;
	}

  	fpeRaised=feclearexcept(FE_ALL_EXCEPT);

	return ret;

}

/* 
	based on
 underlying S, strike K, risk-free rate r, volatility of
 underlying sigma and time to maturity T
*/
double call_price(const double S, const double K, const double r, const double v, const double T) 
{
	double ret= S * norm_cdf(d_j(1, S, K, r, v, T))-K*exp(-r*T) * norm_cdf(d_j(2, S, K, r, v, T));
	int fpeRaised=0;

	fpeRaised=fetestexcept(FE_INVALID | FE_DIVBYZERO |FE_OVERFLOW | FE_UNDERFLOW | FE_INEXACT);

  	if(fpeRaised!=32 && fpeRaised!=0)
	{
		DEBUG("error here!");
   		perror("Log: \n");
		ret = 0;
	}

  	fpeRaised=feclearexcept(FE_ALL_EXCEPT);

	return ret;

}


double call_delta(const double S, const double K, const double r, const double v, const double T) 
{
	double ret = norm_cdf(d_j(1, S, K, r, v, T));

	int fpeRaised=0;

	fpeRaised=fetestexcept(FE_INVALID | FE_DIVBYZERO |FE_OVERFLOW | FE_UNDERFLOW | FE_INEXACT);

  	if(fpeRaised!=32 && fpeRaised!=0)
	{
		DEBUG("error here!");
   		perror("Log: \n");
		ret = 0;
	}

  	fpeRaised=feclearexcept(FE_ALL_EXCEPT);

	return ret;
}


double call_gamma(const double S, const double K, const double r, const double v, const double T) 
{
  	double ret = norm_pdf(d_j(1, S, K, r, v, T))/(S*v*sqrt(T));

	int fpeRaised=0;

	fpeRaised=fetestexcept(FE_INVALID | FE_DIVBYZERO |FE_OVERFLOW | FE_UNDERFLOW | FE_INEXACT);

  	if(fpeRaised!=32 && fpeRaised!=0)
	{
		DEBUG("error here!");
   		perror("Log: \n");
		ret = 0;
	}

  	fpeRaised=feclearexcept(FE_ALL_EXCEPT);

	return ret;
}


double call_vega(const double S, const double K, const double r, const double v, const double T) 
{	
	double ret = S*norm_pdf(d_j(1, S, K, r, v, T))*sqrt(T);

	int fpeRaised=0;

	fpeRaised=fetestexcept(FE_INVALID | FE_DIVBYZERO |FE_OVERFLOW | FE_UNDERFLOW | FE_INEXACT);

  	if(fpeRaised!=32 && fpeRaised!=0)
	{
		DEBUG("error here!");
   		perror("Log: \n");
		ret = 0;
	}

  	fpeRaised=feclearexcept(FE_ALL_EXCEPT);

	return ret;
}


double call_theta(const double S, const double K, const double r, const double v, const double T) 
{
  	double ret = -(S*norm_pdf(d_j(1, S, K, r, v, T))*v)/(2*sqrt(T)) - r*K*exp(-r*T)*norm_cdf(d_j(2, S, K, r, v, T));

	int fpeRaised=0;

	fpeRaised=fetestexcept(FE_INVALID | FE_DIVBYZERO |FE_OVERFLOW | FE_UNDERFLOW | FE_INEXACT);

  	if(fpeRaised!=32 && fpeRaised!=0)
	{
		DEBUG("error here!");
   		perror("Log: \n");
		ret = 0;
	}

  	fpeRaised=feclearexcept(FE_ALL_EXCEPT);

	return ret;

}


double call_rho(const double S, const double K, const double r, const double v, const double T) 
{
  	double ret = K*T*exp(-r*T)*norm_cdf(d_j(2, S, K, r, v, T));

	int fpeRaised=0;

	fpeRaised=fetestexcept(FE_INVALID | FE_DIVBYZERO |FE_OVERFLOW | FE_UNDERFLOW | FE_INEXACT);

  	if(fpeRaised!=32 && fpeRaised!=0)
	{
		DEBUG("error here!");
   		perror("Log: \n");
		ret = 0;
	}

  	fpeRaised=feclearexcept(FE_ALL_EXCEPT);

	return ret;
}

/*
 based on
 underlying S, strike K, risk-free rate r, volatility of
underlying sigma and time to maturity T
*/
double put_price(const double S, const double K, const double r, const double v, const double T) 
{
  	double ret= -S*norm_cdf(-d_j(1, S, K, r, v, T))+K*exp(-r*T) * norm_cdf(-d_j(2, S, K, r, v, T));

	int fpeRaised=0;

	fpeRaised=fetestexcept(FE_INVALID | FE_DIVBYZERO |FE_OVERFLOW | FE_UNDERFLOW | FE_INEXACT);

  	if(fpeRaised!=32 && fpeRaised!=0)
	{
		DEBUG("error here!");
   		perror("Log: \n");
		ret = 0;
	}

  	fpeRaised=feclearexcept(FE_ALL_EXCEPT);

	return ret;
}


double put_delta(const double S, const double K, const double r, const double v, const double T) 
{
  	double ret= norm_cdf(d_j(1, S, K, r, v, T)) - 1;

	int fpeRaised=0;

	fpeRaised=fetestexcept(FE_INVALID | FE_DIVBYZERO |FE_OVERFLOW | FE_UNDERFLOW | FE_INEXACT);

  	if(fpeRaised!=32 && fpeRaised!=0)
	{
		DEBUG("error here!");
   		perror("Log: \n");
		ret = 0;
	}

  	fpeRaised=feclearexcept(FE_ALL_EXCEPT);

	return ret;
}


double put_gamma(const double S, const double K, const double r, const double v, const double T) 
{
  	double ret = call_gamma(S, K, r, v, T); // Identical to call by put-call parity

	int fpeRaised=0;

	fpeRaised=fetestexcept(FE_INVALID | FE_DIVBYZERO |FE_OVERFLOW | FE_UNDERFLOW | FE_INEXACT);

  	if(fpeRaised!=32 && fpeRaised!=0)
	{
		DEBUG("error here!");
   		perror("Log: \n");
		ret = 0;
	}

  	fpeRaised=feclearexcept(FE_ALL_EXCEPT);

	return ret;

}


double put_vega(const double S, const double K, const double r, const double v, const double T) 
{
  	double ret= call_vega(S, K, r, v, T); // Identical to call by put-call parity
	
	int fpeRaised=0;

	fpeRaised=fetestexcept(FE_INVALID | FE_DIVBYZERO |FE_OVERFLOW | FE_UNDERFLOW | FE_INEXACT);

  	if(fpeRaised!=32 && fpeRaised!=0)
	{
		DEBUG("error here!");
   		perror("Log: \n");
		ret = 0;
	}

  	fpeRaised=feclearexcept(FE_ALL_EXCEPT);

	return ret;
}


double put_theta(const double S, const double K, const double r, const double v, const double T) 
{
  	double ret= -(S*norm_pdf(d_j(1, S, K, r, v, T))*v)/(2*sqrt(T))+r*K*exp(-r*T)*norm_cdf(-d_j(2, S, K, r, v, T));

	int fpeRaised=0;

	fpeRaised=fetestexcept(FE_INVALID | FE_DIVBYZERO |FE_OVERFLOW | FE_UNDERFLOW | FE_INEXACT);

  	if(fpeRaised!=32 && fpeRaised!=0)
	{
		DEBUG("error here!");
   		perror("Log: \n");
		ret = 0;
	}

  	fpeRaised=feclearexcept(FE_ALL_EXCEPT);

	return ret;
}


double put_rho(const double S, const double K, const double r, const double v, const double T) 
{
  	double ret = -T*K*exp(-r*T)*norm_cdf(-d_j(2, S, K, r, v, T));

	int fpeRaised=0;

	fpeRaised=fetestexcept(FE_INVALID | FE_DIVBYZERO |FE_OVERFLOW | FE_UNDERFLOW | FE_INEXACT);

  	if(fpeRaised!=32 && fpeRaised!=0)
	{
		DEBUG("error here!");
   		perror("Log: \n");
		ret = 0;
	}

  	fpeRaised=feclearexcept(FE_ALL_EXCEPT);

	return ret;
}



/* Note is for European Vanilla options
S = Option price
K = Strike price
r = Risk-free rate 
v = Volatility
T = One year until expiry 
*/
void create_greek_table(struct mg_connection *c,char *name,char *type,double S, double K, double r, double v, double T) 
{
	double breakeven=0; // BIP http://www.investopedia.com/terms/b/breakevenpoint.asp

	if(!strncmp(type,"CALL",4)) 
  	{
		char type_call[1024];	
  		double call = call_price(S, K, r, v, T);
  		double call_delta_v = call_delta(S, K, r, v, T);
  		double call_gamma_v = call_gamma(S, K, r, v, T);
  		double call_vega_v = call_vega(S, K, r, v, T);
  		double call_theta_v = call_theta(S, K, r, v, T);
  		double call_rho_v = call_rho(S, K, r, v, T);
// TODO study Speed, Zomma, Color, Ultima... https://en.wikipedia.org/wiki/Greeks_%28finance%29#Third-order_Greeks WTF oh GOD	
		breakeven=K+call;  
	
  		memset(type_call,0,1024);

  		snprintf(type_call,1024,"{\"name\":\"%s\",\"type\":\"%s\",\"premium\":\"%f\",\"breakeven\":\"%f\",\"strike\":\"%f\",\"delta\":\"%f\",\"gamma\":\"%f\",\"vega\":\"%f\",\"theta\":\"%f\",\"rho\":\"%f\"}",name,"CALL",call,breakeven,K,call_delta_v,call_gamma_v,call_vega_v,call_theta_v,call_rho_v); 

		mg_send_websocket_frame(c, WEBSOCKET_OP_TEXT, type_call, 1024);

	} else {

		char type_put[1024];	
  		double put = put_price(S, K, r, v, T);
  		double put_delta_v = put_delta(S, K, r, v, T);
  		double put_gamma_v = put_gamma(S, K, r, v, T);
  		double put_vega_v = put_vega(S, K, r, v, T);
  		double put_theta_v = put_theta(S, K, r, v, T);
  		double put_rho_v = put_rho(S, K, r, v, T);

		breakeven=K-put; 

  		memset(type_put,0,1024);

		snprintf(type_put,1024,"{\"name\": \"%s\",\"type\":\"%s\",\"premium\": \"%f\",\"breakeven\":\"%f\",\"strike\": \"%f\",\"delta\": \"%f\",\"gamma\": \"%f\",\"vega\": \"%f\",\"theta\": \"%f\",\"rho\": \"%f\"}",name,"PUT",put,breakeven,K,put_delta_v,put_gamma_v,put_vega_v,put_theta_v,put_rho_v); 	
		mg_send_websocket_frame(c, WEBSOCKET_OP_TEXT, type_put, 1024);
	}
  

}
