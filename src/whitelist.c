#include "whitelist.h"

// test IP using whitelist
bool whitelist_ip(char * addr)
{
	FILE * arq;
	bool at_list=false;

	arq = fopen("config/whitelist.conf", "r");

	if( arq == NULL )
	{

		DEBUG("error to open() file"); 	 
		exit(1);
	}

	char line[128];
	memset(line,0,128);

	while( fgets(line,sizeof line,arq) && at_list==false )
	{
		line[strlen(line)-1]='\0';

// the whitelist can accept regex: 127.0.0.* ...
		if(strstr(addr,line))
			at_list=true;
	}

	if( fclose(arq) == EOF )
	{
		DEBUG("Error in close() conf/whitelist.conf ");
		exit(1);
	}

	arq=NULL;

	return at_list;
}



