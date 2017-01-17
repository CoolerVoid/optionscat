#include "compound.h"
#include "../lib/libmongoose/mongoose.h"
#include "../lib/BSD/strsec.h"

// return json with compound interest of years
void calc_compound_interest(struct mg_connection *c,double principle, double rate, double times) 
{

    	double amount=0, interest=0; 
	int fpeRaised=0;
    	char *json_interest=NULL;
    	char tmp[21];
    	size_t len=1;

// TODO improve this limit, to return debug error..

	if(times > 50 || times < 2)
	{
		DEBUG("error at limit");
		times=5;
	}

	json_interest=strdup("{");	  

		while(times)
		{	 
			memset(tmp,0,21);
			
    			amount = principle*pow(1+(rate/100),times); 
			fpeRaised=fetestexcept(FE_INVALID | FE_DIVBYZERO |FE_OVERFLOW | FE_UNDERFLOW | FE_INEXACT);

  			if(fpeRaised!=32 && fpeRaised!=0)
			{
				DEBUG("error here!");
   				perror("Log: \n");
    				XFREE(json_interest);
  				fpeRaised=feclearexcept(FE_ALL_EXCEPT);
				return;
			}

  			fpeRaised=feclearexcept(FE_ALL_EXCEPT);

    			interest = amount-principle;
			snprintf(tmp,21,"\"%1.f\":\"%.f\", ",times,interest);
			len += strlen(tmp);
			json_interest = xrealloc(json_interest,len);
			strlcat(json_interest,tmp,len); 
			times--; 
		}

	*(json_interest+(strlen(json_interest)-2))='\0';
	json_interest=xrealloc(json_interest,len);
	strlcat(json_interest,"}",len); 
    	mg_send_websocket_frame(c, WEBSOCKET_OP_TEXT, json_interest, len-2);

    	XFREE(json_interest);
	
}
