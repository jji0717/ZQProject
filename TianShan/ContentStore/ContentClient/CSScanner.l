%{

#include "ZQ_common_conf.h"
#include <cstring>

#ifdef ZQ_OS_LINUX
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "CSGrammer.hpp"
#include "ContentClient.h"

bool isEOF = false;

extern void keepNum();
extern void keepStr();
extern void expandVar(const char*, char*);
extern CSClient client;

#ifdef ZQ_OS_LINUX
#define YY_INPUT(buf,result,max_size) \
	if ( yy_current_buffer->yy_is_interactive ) \
		{ \
        const char* prompt = client.prompt().c_str(); \
        char* tmp = readline(prompt); \
        if(tmp && *tmp) { \
            strncpy(buf, tmp, strlen(tmp)); \
            buf[strlen(tmp)] = '\n'; \
            add_history(tmp); \
            result = strlen(tmp)+1; \
        } \
        else if(!*tmp) { \
            buf[0] = '\n'; \
            result = 1; \
        } \
        else { \
			YY_FATAL_ERROR( "input in flex scanner failed" ); \
            result = 0; \
		} \
        } \
	else if ( ((result = fread( buf, 1, max_size, yyin )) == 0) \
		  && ferror( yyin ) ) \
		YY_FATAL_ERROR( "input in flex scanner failed" );
#endif

%}

%option noyywrap

%%

<<EOF>>   { isEOF = true; return (0); }

help      { return HELP; }
close     { return CLOSE; }
clear     { return CLEAR; }
sleep     { return SLEEP; }
connect   { return CONNECT; }

open      { return OPEN; }
list      { return LIST; }
expose    { return EXPOSE; }
info      { return INFO; }
current   { return CURR; }
volume    { return VOLUME; }
folder    { return FOLDER; }

destroy   { return DESTROY; }
provision { return PROV; }
cancel    { return CANCEL; }

update    { return UPDATE; }

mset      { return MSET; }
pset      { return PSET; }
sync      { return SYNC; }
set       { return SET; }

export	  { return EXPORT; }
cache     { return CACHE; }
hashFolder	  { return HASHFOLDER; }
hashFolderName {return HASHFOLDERNAME;}

up        { return UP; }
timer     { return TIMER; }
start     { return TSTART; }
stop      { return TSTOP; }
sleep     { return SLEEP; }
addAccessCount  { return ADDACCESSCOUNT;}
accessCount  { return ACCESSCOUNT;}
listMissed  { return LISTMISSED; }
listHot     { return LISTHOT; }
setCacheWindow { return SETCACHEWINDOW;}
cacheWindow { return CACHEWINDOW;}
storeDistance	 { return STOREDISTANCE;}
localFn  { return LOCALFN;}

[!].+           { return SYSCOMMAND;}

"exit"|"quit" { return EXIT; }

"true"|"false" { 
		   	if(!strcmp(yytext, "true")) {
				yylval.val = 1;
				return TF;
			}
			else if(!strcmp(yytext, "false")) {
				yylval.val = 0;
				return TF; 
			}
			else {}
		  }

[0-9]{1}[0-9]*    { keepNum(); return INTEGER; }

=         { return EQ; }

"ftp"|"cifs"|"nfs" { keepStr(); return EXPROTO; }
		  
			
[a-zA-Z]{3,4}"://"([a-zA-Z0-9\.\-_]+":"[a-zA-Z0-9]+"@")?[a-zA-Z0-9_\.]+(:[0-9]+)?("/"[a-zA-Z0-9\-_/\$\.]*)*  {
				keepStr();
                return URL;
			 }

\" {
	/* reading until next quote */
	char str[128]; 
	char* p = str;
	
	int c = 0;
	bool closed = false;
	do {
		c = yyinput();
		if(c == '"') {
			if(*(p-1) == '\\') {
				*(p-1) = c;
				continue;
			}
			*p++ = '\0';
			closed = true;
			break;	
		}
		*p++ = c;
	} while(c != '\n' && c != EOF);
	
	if(closed) {
		char res[512];
		memset(res, '\0', 256);
		expandVar(str, res);
		
		short len = strlen(res);
		yylval.str = new char[len+1];
		strncpy(yylval.str, res, len+1);
		
		return NAME;
	}
	return NL;
  }             
             
[0-9a-zA-Z\-_\.\*/,\$:'\+%]+ { keepStr(); return NAME; }
            
\n          { return NL; }

[ \t\v]   ;

.         ;


%%

void keepNum() {
	char res[256];
	memset(res, '\0', 256);
	
	expandVar(yytext, res);
	yylval.val = atoi(res);
}

void keepStr() {
	char res[256];
	memset(res, '\0', 256);
	
	expandVar(yytext, res);
	short len = strlen(res);
	
	yylval.str = new char[len+1];	
	strncpy(yylval.str, res, len+1);
}

void expandVar(const char* str, char* res) {
	if(!str || !str[0]) {
		return;
	}
	
	const char* src = str;
	
    while(src) {
        const char* p1 = strchr(src, '%');

        if(!p1) {
            strcat(res, src);
            break;
        }

        const char* p2 = strchr(p1+1, '%');

        if(!p2) {
            strcat(res, src);
            break;
        }

        /* found the pair */
        short len = p2-p1-1;

        char* var = new char[len+1];
        strncpy(var, p1+1, len);
        var[len] = '\0';

		strncat(res, src, p1-src);
	    char* env = getenv(var);
        if(env) {
			strncat(res, env, strlen(env));
        }
        else {
			strncat(res, p1, p2-p1+1);
        }
		delete[] var;      
		
		src = p2 + 1;
    }

    if(src==str) {
        return;
    }
}

