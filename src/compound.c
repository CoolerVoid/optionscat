#include "compound.h"
#include "../lib/libmongoose/mongoose.h"
#include "../lib/BSD/strsec.h"
#include "mem_ops.h"

// return json with compound interest of years
void calc_compound_interest(struct mg_connection *c,double principle, double rate, double time) 
{

    	double amount=0, interest=0; 
    	char *json_interest=NULL;
    	char tmp[21];
    	size_t len=1;
// TODO improve this limit, to return debug error..

	if(time>50 || time < 2)
	{
		DEBUG("error at limit");
		time=5;
	}

	json_interest=strdup("{");	  

		while(time)
		{	 
			memset(tmp,0,20);
			tmp[20]='\0';	
    			amount = principle*pow(1+(rate/100),time); 
    			interest = amount-principle;
			snprintf(tmp,21,"\"%1.f\":\"%.f\", ",time,interest);
			len += strlen(tmp);
			json_interest = xrealloc(json_interest,len);
			strlcat(json_interest,tmp,len); 
			time--; 
		}

	*(json_interest+(strlen(json_interest)-2))='\0';
	json_interest=xrealloc(json_interest,len);
	strlcat(json_interest,"}",len); 

    mg_send_websocket_frame(c, WEBSOCKET_OP_TEXT, json_interest, len-2);

//    printf("debug compound: %s\n",json_interest);	
    XFREE(json_interest);
	
}
