#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/resource.h>
#include <stdbool.h>
#include "validate.h"   // to validate inputs
#include "greeks.h"   // calculate European vanilla options
#include "html_entities.h" // o make XSS mitigation
#include "mem_ops.h" // xmalloc() ,XFREE()...
#include "../lib/BSD/strsec.h" // strlcpy(), strlcat() and strnstr()
#include "../lib/frozen/frozen.h" // json parser
#include "../lib/libmongoose/mongoose.h" // HTTPd lib + krypton
#include "whitelist.h" // list os whitelist to access this server, file  "conf/whitelist.conf"
#include "compound.h"


static sig_atomic_t s_signal_received = 0;
static struct mg_serve_http_opts s_http_server_opts;

static void signal_handler(int sig_num) 
{
  signal(sig_num, signal_handler);  
  s_signal_received = sig_num;
}

static int is_websocket(const struct mg_connection *nc) 
{
  return nc->flags & MG_F_IS_WEBSOCKET;
}

static void broadcast(struct mg_connection *nc, const struct mg_str msg) 
{
	if(msg.p==NULL)
		return;

  size_t sizeaction=0;
  struct mg_connection *c;
  short blackscholes=1,compound=1,counter_json=0;
  char buf[4048];
  char *action, *strike, *name, *volatility, *under, *risk, *maturity, *type, *csrf_token, *value, *years, *percent;

	memset(buf,0,4048);
	
	if(msg.len>4047)
		return;

	if( msg.p )
	{

	  	snprintf(buf, sizeof(buf), "%.*s", (int) msg.len, msg.p);

// parse json
		counter_json=json_scanf(buf, strlen(buf), "{ action:%Q, strike:%Q, name:%Q, volatility:%Q, under:%Q, risk:%Q, maturity:%Q , csrf_token:%Q, type:%Q, value:%Q, years:%Q, percent:%Q }", &action, &strike, &name, &volatility, &under, &risk, &maturity,&csrf_token,&type,&value,&years,&percent);

	        if( action && counter_json>=2  )	
			sizeaction=strnlen(action,12);
		else
			sizeaction=0;



  		for ( c = mg_next(nc->mgr, NULL); c != NULL && counter_json>=2; c = mg_next(nc->mgr, c) ) 
		{

  			
//printf("TESTE2\n %s %s %s\n",type,name,action);				
// this get information from form at index.html, if user click in "start"
				if(action && strnstr(action,"blackscholes",sizeaction) && blackscholes==1)
  				{

  					double S=0,K=0,r=0,v=0,T=0;

// validate length of input here...
					if(validate_bs(strike,volatility,under,risk,maturity)==false)					
						goto FREE_scholes;

					blackscholes=0;	
// convert string to double...
					K=strtod(strike,(char **)NULL);	
					v=strtod(volatility,(char **)NULL);		
					S=strtod(under,(char **)NULL);		
					r=strtod(risk,(char **)NULL);		
					T=strtod(maturity,(char **)NULL);		
							
/* Note is for European Vanilla options
S= Option price
K= Strike price
r= Risk-free rate (5%)
v= Volatility of the underlying (20%)
T= One year until expiry
*/
					create_greek_table(c,name,type,S,K,r,v,T);

					FREE_scholes:
					XFREE(strike);
					XFREE(name);
					XFREE(under);	
					XFREE(volatility);
					XFREE(maturity);
					XFREE(risk);
					blackscholes=0;	
				}
// calc compound interest
				if(action && strnstr(action,"compound",sizeaction) && compound==1)
  				{

					double v=0,y=0,p=0;

					
					if(validate_compound(value,years,percent)==false)					
						goto FREE_compound;


					v=strtod(value,(char **)NULL);
					y=strtod(years,(char **)NULL);
					p=strtod(percent,(char **)NULL);
// TODO need validate this
					calc_compound_interest(c,v,p,y);				

					FREE_compound:	
					XFREE(value);
					XFREE(years);
					XFREE(percent);
					compound=0;
				}

			
			XFREE(action);
  		}
    	}
}






static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) 
{

	switch (ev) 
	{	
		case MG_EV_HTTP_REQUEST: {

			char addr[32];
  			mg_sock_addr_to_str(&nc->sa, addr, sizeof(addr), MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT);

			if(whitelist_ip(addr)==true)
				mg_serve_http(nc, (struct http_message *)ev_data, s_http_server_opts);
      			break;
    		}
	
    		case MG_EV_WEBSOCKET_HANDSHAKE_DONE: {
      			broadcast(nc, mg_mk_str("++ joined"));
      			break;
    		}

    		case MG_EV_WEBSOCKET_FRAME: {
			char addr[32];
  			mg_sock_addr_to_str(&nc->sa, addr, sizeof(addr), MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT);
// config white list at conf/whitelist.conf
				if(whitelist_ip(addr)==true)
				{
      					struct websocket_message *wm = (struct websocket_message *) ev_data;
      					struct mg_str d = {(char *) wm->data, wm->size};
      					broadcast(nc, d);
				}
      			break;
    			}
	
    		case MG_EV_CLOSE: {
      			if (is_websocket(nc)) 
     				broadcast(nc, mg_mk_str("-- left"));
     			break;
	
    			}
  	}
}
	
int main()
{
	char port[]="1347";
	struct mg_mgr mgr;
	struct mg_connection *nc;

	signal(SIGTERM, signal_handler);
	signal(SIGINT, signal_handler);
	setvbuf(stdout, NULL, _IOLBF, 0);
	setvbuf(stderr, NULL, _IOLBF, 0);

	mg_mgr_init(&mgr, NULL);

	nc = mg_bind(&mgr, port, ev_handler);
	mg_set_protocol_http_websocket(nc);
/* if run with TLS... 
 * cert+pem both in pem file
 * */ 
  	const char *err_str = mg_set_ssl(nc, "cert/certkey.pem", NULL);

    		if (err_str != NULL) 
		{
      			fprintf(stderr, "Error loading SSL cert: %s\n", err_str);
      			exit(1);
    		}

  	s_http_server_opts.document_root = "web/";
  	s_http_server_opts.dav_document_root = "web/";  // Allow access via WebDav
  	s_http_server_opts.enable_directory_listing = "no";

  	fprintf(stdout,"OPTIONSCAT v0.01 server\n started on port %s\n", port);

  		while (s_signal_received == 0) 
  			mg_mgr_poll(&mgr, 2000);
  
  	mg_mgr_free(&mgr);

	exit(0);
}

