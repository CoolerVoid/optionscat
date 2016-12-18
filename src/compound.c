#include "compound.h"
#include "../lib/libmongoose/mongoose.h"
#include "../lib/BSD/strsec.h"

// return json with compound interest of years
void calc_compound_interest(struct mg_connection *c,double principle, double rate, double times) 
{

    	double amount=0, interest=0; 
    	char *json_interest=NULL;
    	char tmp[21];
    	size_t len=1;
// TODO improve this limit, to return debug error..

	if(times>50 || times < 2)
	{
		DEBUG("error at limit");
		times=5;
	}

	json_interest=strdup("{");	  

		while(times)
		{	 
			memset(tmp,0,20);
			tmp[20]='\0';	
    			amount = principle*pow(1+(rate/100),times); 
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

//    printf("debug compound: %s\n",json_interest);	
    XFREE(json_interest);
	
}
