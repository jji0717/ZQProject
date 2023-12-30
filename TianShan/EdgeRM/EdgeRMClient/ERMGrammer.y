%{

#include "EdgeRMClient.h"
#include <iostream>
#include <sstream>

extern int yylex(void);
extern void yyerror(char*);
extern char* yytext;
extern bool isEOF;

EdgeRMClient client;

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
%token CONNECT OPEN LIST SET MSET PSET INFO CURR DESTROY CANCEL PROV EXPOSE ADD ADDCHANNEL REMOVE UPDATE POPULATE CREATE ENABLE IMPORT EXPORT LINK UNLINK IMPORTROUTES EXPORTROUTES

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
	| OPEN INTEGER						{ client.openDevice(toString($2)); }
	| OPEN NAME							{ client.openDevice($2); }
	| IMPORT NAME   					{ client.importDevice($2); }
	| IMPORT NAME NAME NAME INTEGER INTEGER	{ client.importDevice($2, $3, $4, $5, $6); }
	| IMPORTROUTES NAME                 { client.importRoutes($2);}
	| EXPORTROUTES NAME NAME            { client.exportRoutes($2,$3);}
	| EXPORT NAME NAME		{ client.exportDevice($2, $3);}	
	| OPEN ALLOCATION INTEGER			{ client.openAllocation(toString($3)); }
	| OPEN ALLOCATION NAME				{ client.openAllocation($3); }
	| OPEN CHANNEL INTEGER INTEGER		{ client.openChannel($3, $4); }
	| OPEN CHANNEL INTEGER				{ client.openPort($3); }
	| OPEN CHANNEL NAME					{ client.openChannel($3); }
	| OPEN PORT INTEGER					{ client.openPort($3); }
	| ADD NAME							{ client.addDevice($2); }
    | ADDCHANNEL INTEGER INTEGER INTEGER INTEGER INTEGER INTEGER INTEGER INTEGER INTEGER INTEGER         { client.addChannel($2,$3,$4,$5,$6,$7,$8,$9,$10,$11);}	
	| CREATE ALLOCATION INTEGER			{ client.createAllocation(toString($3)); }
	| CREATE ALLOCATION NAME			{ client.createAllocation($3); }
	| ADD PORT INTEGER					{ client.addPort($3); }
	| POPULATE INTEGER					{ client.populateChannel($2); }
	| UPDATE NAME						{ client.updateDevice($2); }
	| UPDATE PORT INTEGER				{ client.updatePort($3); }	
	| REMOVE INTEGER					{ client.removeDevice(toString($2)); }
	| REMOVE NAME						{ client.removeDevice($2); }
	| REMOVE ALLOCATION INTEGER			{ client.removeAllocation(toString($3)); }
	| REMOVE ALLOCATION NAME			{ client.removeAllocation($3); }
	| ENABLE CHANNEL INTEGER			{ client.enableChannel($3); }
	| UPDATE CHANNEL					{ client.updateChannel(); }
	| REMOVE PORT INTEGER				{ client.removePort($3); }
	| LIST  							{ client.listDevice(); }
	| LIST CHANNEL INTEGER				{ client.listChannel($3); }
	| LIST ALLOCATION					{ client.listAllocation(); }
	| LIST PORT							{ client.listPort(); }
	| INFO								{ client.info(); }
	| CURR								{ client.current(); }
	| SET NAME EQ INTEGER				{ client.setProperty($2, toString($4)); }
	| SET NAME EQ NAME					{ client.setProperty($2, $4); }
	| LINK INTEGER NAME NAME			{ client.linkRouteName($2, $3, $4); }
	| LINK INTEGER NAME INTEGER	 	{ client.linkRouteName($2, $3, $4); }
	| LINK NAME NAME					{ client.linkRouteName($2, $3); }
	| UNLINK INTEGER NAME			{ client.unlinkRouteName($2, $3); }
	| UNLINK NAME					{ client.unlinkRouteName($2); }
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


