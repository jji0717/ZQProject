%{

#include <cstring>
#include "ContentLibGrammer.hpp"
#include <ContentLibClient.h>

bool isEOF = false;

extern void keepNum();
extern void keepStr();
extern void expandVar(const char*, char*);

%}

%option noyywrap

%%

<<EOF>>		{ isEOF = true; return (0); }

help		{ return HELP; }
close		{ return CLOSE; }
clear		{ return CLEAR; }
connect		{ return CONNECT; }

open		{ return OPEN; }
list		{ return LIST; }
info		{ return INFO; }
current		{ return CURR; }
volume		{ return VOLUME; }
content	    { return CONTENT; };

set			{ return SET; }
reset       { return RSET; }

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


[1-9]{1}[0-9]*    { keepNum(); return INTEGER; }

=         { return EQ; }

"ftp"|"cifs"|"nfs" { keepStr(); return EXPROTO; }
		  
			
[a-zA-Z]{3,4}"://"([a-zA-Z0-9\.\-_]+":"[a-zA-Z0-9]+"@")?[a-zA-Z0-9_\.]+(:[0-9]+)?("/"[a-zA-Z0-9\-_/\$\.]*)*  {
				keepStr();
                return URL;
			 }

\" {
	/* reading until next quote */
	char str[512]; 
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
		char res[1024];
		memset(res, '\0', 1024);
		expandVar(str, res);
		
		short len = strlen(res);
		yylval.str = new char[len+1];
		strncpy(yylval.str, res, len+1);
		
//		printf("var substitution: %s\n", yylval.str);
		return NAME;
	}
	return NEWLINE;
  }             
             
[0-9a-zA-Z\-_\.\*/,\$:'\+%]+ { keepStr(); return NAME; }
            
\n          { return NEWLINE; }

[ \t\v]   ;

.         ;


%%

void keepNum() {
	char res[1024];
	memset(res, '\0', 1024);
	
	expandVar(yytext, res);
	yylval.val = atoi(res);
}

void keepStr() {
	char res[1024];
	memset(res, '\0', 1024);
	
	expandVar(yytext, res);
	short len = strlen(res);
	
	yylval.str = new char[len+1];	
	strncpy(yylval.str, res, len+1);
	
//	printf("var substitution: %s\n", yylval.str);
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

