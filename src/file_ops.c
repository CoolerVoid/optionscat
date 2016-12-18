#include "file_ops.h"
#include "mem_ops.h"
#include "../lib/libmongoose/mongoose.h"
#include "html_entities.h"
#include "../lib/BSD/strsec.h"
#include "validate.h"
#define MAX_file_len 1000000

char *file_content(const char * filename) 
{
    FILE *fp = fopen(filename, "r");
    size_t file_size;
    long pos;
    char *file_contents;

    	if (!fp)
        	return NULL;
    	fseek(fp, 0L, SEEK_END);
    	pos = ftell(fp);

    	if (pos < 0) 
	{
        	fclose(fp);
        	return NULL;
    	}

    	file_size = pos;
    	rewind(fp);
    	file_contents = xmalloc(sizeof(char) * (file_size + 1));

    	if (!file_contents) 
	{
        	fclose(fp);
        	return NULL;
    	}

    	if (fread(file_contents, file_size, 1, fp) < 1) 
	{
        	if (ferror(fp)) 
		{
            		fclose(fp);
            		XFREE(file_contents);
            		return NULL;
        	}
    	}

   	fclose(fp);
    	file_contents[file_size] = '\0';

    	return file_contents;
}

//read lines of file
char *ReadLines(char * NameFile)
{
	FILE * fh;
	static char buffer[MAX_file_len];

	memset(buffer,0,MAX_file_len-1);

	fh = fopen(NameFile, "rb");

	if( fh == NULL )
	{

		DEBUG("error in to open() file");
		perror("Error ");
		exit(-1); 	 
		
	}

	if(fseek(fh, 0L, SEEK_END)==0)
	{
    		size_t s = ftell(fh);
    		rewind(fh);

    		if ( buffer != NULL && s < MAX_file_len )
    		{
      			if(!fread(buffer, s, 1, fh))
				DEBUG("error \n");
    		}
	}

 
	if( fclose(fh) == EOF )
	{
		DEBUG("Error in close() file %s",NameFile);
		exit(1);
	}

	fh=NULL;

	char *tmp=buffer;
	
	return tmp;
}



// write string in file
void Write_File(char *file,char *str)
{
	FILE *arq;
 
	arq=fopen(file,"w"); 

	if ( arq == NULL ) 
	{
//		fclose(arq);
		DEBUG("error in WriteFile() %s",file); 
		exit(1);
	}

	fprintf(arq,"%s",str); 

	if( fclose(arq) == EOF )
	{
		DEBUG("error in Write() file %s",file);
		exit(1);
	}
	arq=NULL;
}
