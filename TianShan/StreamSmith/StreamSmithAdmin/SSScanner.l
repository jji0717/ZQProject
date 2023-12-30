%{

#include <cstring>
#include "SSGrammer.hpp"

bool isEOF = false;

extern void keepNum(bool fraction=false);
extern void keepStr();
extern void expandVar(const char*, char*);

%}

%option noyywrap

IP [0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}
DIGIT -?[1-9]{1}[0-9]*
FRACTION -?[0-9]+\.[0-9]+

%%

<<EOF>> { isEOF = true; return(0); }

help { return HELP; }
"exit"|"quit" { return EXIT; }

connect { return CONNECT; }
close { return DISCON; }

verbose { return VERBOSE; }
"true"|"false" { 
   	if(!strcmp(yytext, "true")) {
		yylval.val = 1;
	}
	else if(!strcmp(yytext, "false")) {
		yylval.val = 0;
	}
	else {
		yylval.val = -1;
	}
	return TF;
}

"begin"|"end" {
	if(!strcmp(yytext, "begin")) {
		yylval.val = 1;
	}
	else if(!strcmp(yytext, "end")) {
		yylval.val = 2;
	}
	else {
		yylval.val = 0;
	}
	return POS;		
}

list { return LIST; }
item { return ITEM; }
streamer { return STREAMER; }

create { return CREATE; }
{IP}   { keepStr(); return ADDR; }

push { return PUSH; }
{DIGIT} { keepNum(); return INTEGER; }
erase { return ERASE; }
destroy { return DESTROY; }
all { return ALL; }

play { return PLAY; }
pause { return PAUSE; }
resume { return RESUME; }
seek { return SEEK; }
seekItem { return SEEKITEM; }
speed { return SPEED; }
rate { return RATE; }
{FRACTION} { keepNum(true); return FRAC; }

current { return CURR; }
select { return SELECT; }
alias { return ALIAS; }
info { return INFO; }
macaddr { return MACADDR; }

clear { return CLEAR; }

\" {
	/* reading until next quote */
	char str[64]; 
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
		char res[256];
		memset(res, '\0', 256);
		expandVar(str, res);
		
		short len = strlen(res);
		yylval.str = new char[len+1];
		strncpy(yylval.str, res, len+1);
		
//		printf("var substitution: %s\n", yylval.str);
		return STR;
	}
	return NL;
}

[0-9a-zA-Z\-_\.\*/,\$:'\+%\\]+ { keepStr(); return STR; }

\n { return NL; }
[ \t\v] ;
. ;

%%

void keepNum(bool fraction) {
	char res[256];
	memset(res, '\0', 256);
	
	expandVar(yytext, res);
	if(fraction) {
		yylval.frac = atof(res);
	}
	else {
		yylval.val = atoi(res);
	}
}

void keepStr() {
	char res[256];
	memset(res, '\0', 256);
	
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