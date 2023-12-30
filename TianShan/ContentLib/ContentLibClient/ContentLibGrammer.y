%{

#include "ContentLibClient.h"
#include <iostream>
#include <sstream>

extern int yylex(void);
extern void yyerror(char*);
extern char* yytext;
extern bool isEOF;

ContentLibClient client;

extern std::string toString(int);

#ifdef ZQ_OS_MSWIN
#define CLS "cls"
#else
#define CLS "clear"
#endif

%}

%union {
	int val;
	char* str;
};

%token HELP EXIT CLOSE CLEAR NONE
%token CONNECT OPEN LIST SET MSET PSET INFO CURR DESTROY CANCEL PROV CONTENT VOLUME RSET

%token <val> CHANNEL ALLOCATION INTEGER TF NEWLINE EQ PORT
%token <str> URL ENDPOINT NAME EXPROTO

%type <val> cmd 

%% 

prog: NEWLINE      { return (0); } 
	| cmd NEWLINE  { return (0); }
	;

cmd:  HELP								{ client.usage(); }
    | EXIT								{ client.exit(); }      
    | CLOSE								{ client.close(); }
    | CLEAR								{ system(CLS); }
	| CONNECT NAME						{ client.connect($2); }
	| OPEN INTEGER						{ client.toStoreReplica(toString($2)); }
	| OPEN NAME							{ client.toStoreReplica($2); }
	| OPEN VOLUME NAME					{ client.toVolume($3); }
	| OPEN CONTENT NAME					{ client.toContentReplica($3); }
	| INFO								{ client.info(); }
	| CURR								{ client.current(); }
	| SET NAME EQ INTEGER				{ client.setProperty($2, toString($4)); }
	| SET NAME EQ NAME					{ client.setProperty($2, $4); }
	| RSET								{ client.reset(); }
	| LIST								{ client.list(); }	
	| LIST VOLUME					    { client.listVolume(); }
	| LIST CONTENT            			{ client.listContent(); }
	| error								{}
	;
	 
%%

std::string toString(int val) {
	std::ostringstream oss; oss << val;
	return oss.str();
}

void yyerror(char* s) {
	if(!client.isInteractive() && !isEOF) {
		std::cerr << s << " near token (" << yytext << ")" << std::endl;
		return;
	}
	if(client.isInteractive()) {
		std::cerr << "bad command or invalid syntax, use \"help\" for more information: " << std::endl;
	}
}

