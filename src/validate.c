#include "validate.h"

bool path_is_valid(char * path)
{
    struct stat sb;
    char *blacklist[]={"..\\","../","%2e","%5c","%2f","%255c","%252e","%9c","%c\0"};
    short sizelist=0;
    size_t sizepath=strlen(path);

	while(sizelist!=9)
	{
		if(path!=NULL)
			if(strlen(path)>2)
				if(strnstr(path,blacklist[sizelist],sizepath))
					return false;
		sizelist++;
	}

	

	if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode))
		return true;
    	else
		return false;
} 



bool extension_is_valid(char * extension)
{
    char *whitelist[]={".(c|cpp)$",".php$",".cs$",".(jad|java|class)$",".rb$",".(js|html)$",".(asp|aspx)$"};
    short sizelist=0;
    size_t size_extension=strlen(extension);	

	while(sizelist!=9)
	{
		if(extension!=NULL)
			if(strlen(extension)>2)
				if(strncmp(extension,whitelist[sizelist],size_extension))
					return true;
		sizelist++;
	}

	return false;
}



bool module_is_valid(char * module)
{
    char *whitelist[]={"eggs/c_common_failures.egg","eggs/php_common_fail.egg","eggs/csharp_common_fails.egg","eggs/java_common_fail.egg","eggs/ruby_common_fail.egg","eggs/javascript_common_fail.egg","eggs/asp_common_fails.egg"};
    short sizelist=0;
    size_t sizemodule=strlen(module);

	while(sizelist!=9)
	{
		if(module!=NULL)
			if(strlen(module)>5)
				if(strnstr(module,whitelist[sizelist],sizemodule))
					return true;
		sizelist++;
	}

	return false;
} 
 


bool url_viewcode_valid(char* test)
{

 char *p;
 short match_rank=0,counter=0;

 	p = test;

	if(strnlen(test,3064)>3063)
		return false;

 	do{
		if(p[0] == 'p' && p[1] == 'a' && p[2] == 't' && p[3] == 'h' && p[4] == '=')
		{
			p += 5;
			match_rank+=1;	
		}
		p++;
	}while(p[5] != '\0' && !match_rank);

	do{
		p+=1;
		counter+=1;
	}while(p[0]!='&');

	match_rank+=counter>5?1:0;
	counter=0;
 
	do{
		if(p[0] == '&' && p[1] == 'l' && p[2] == 'a' && p[3] == 'n' && p[4] == 'g' && p[5] == '=')
		{
			p += 6;
			match_rank+=1;	
		}
		p++;
	}while(p[6] != '\0' && match_rank==1);

	do{
		p+=1;
		counter+=1;
	}while(p[0]!='&');

	match_rank+=counter>=1?1:0;
	counter=0;

	do{
		if(p[0] == '&' && p[1] == 'l' && p[2] == 'i' && p[3] == 'n' && p[4] == 'e' && p[5] == 's' && p[6] == '=')
		{
			p += 7;
			match_rank+=1;	
		}
		p++;
	}while(p[7] != '\0' && match_rank==2);

	do {
		p+=1;
		counter+=1;
	}while(p[0]!='\0');

	match_rank+=counter>=1?1:0;
 
	if(match_rank==6)
		return true;
	else
		return false;
}
