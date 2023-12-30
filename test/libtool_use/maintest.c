#include <stdio.h> 

 extern int compress_file (const char *filename); 

int main (int argc, char *argv[]) 
{ 
	int ia=100;
	double df=90.90;
	if (argc < 2) 
	{	   
		printf ("usage : %s file\n", argv[0]); 
		return 1; 
	}   
	return compress_file (argv[1]); 
} 

