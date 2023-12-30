%{

#include "ContentClient.h"
#include "SystemUtils.h"
#include <iostream>
#include <sstream>

extern int yylex(void);
extern void yyerror(char*);
extern char* yytext;
extern bool isEOF;

CSClient client;

#ifdef ZQ_OS_MSWIN
#define CLS "cls"
#else
#define CLS "clear"
#endif

extern std::string toString(int);

%}

%union {
	int val;
	char* str;
};

%token HELP EXIT CLOSE CLEAR SLEEP NONE SLEEP ADDACCESSCOUNT ACCESSCOUNT LISTMISSED SETCACHEWINDOW CACHEWINDOW STOREDISTANCE LOCALFN LISTHOT SYSCOMMAND
%token CONNECT OPEN LIST SET MSET PSET SYNC INFO CURR DESTROY CANCEL PROV EXPOSE UPDATE TIMER TSTART TSTOP UP EXPORT CACHE HASHFOLDER HASHFOLDERNAME

%token <val> VOLUME FOLDER INTEGER TF NL EQ 
%token <str> URL ENDPOINT NAME EXPROTO SUBFILE

%type <val> cmd 

%% 

prog: NL      { return (0); } 
	| cmd NL  { return (0); }
	;

cmd:  HELP				     { client.usage(); }
    | EXIT				     { client.exit(); }      
    | CLOSE                  { client.close(); }
    | CLEAR                  { system(CLS); }
	| SLEEP INTEGER          { SYS::sleep($2); } 
	| CONNECT NAME           { client.connect($2); }
	| OPEN INTEGER           { client.openContent(toString($2)); }
	| OPEN INTEGER TF        { client.openContent(toString($2), $3); }
	| OPEN NAME	             { client.openContent($2); }
	| OPEN NAME TF           { client.openContent($2, $3); }
	| OPEN VOLUME INTEGER    { client.openVolume(toString($3)); }
	| OPEN VOLUME NAME       { client.openVolume($3); }
	| OPEN FOLDER INTEGER    { client.openFolder(toString($3)); }
	| OPEN FOLDER INTEGER TF { client.openFolder(toString($3), $4); }
	| OPEN FOLDER NAME       { client.openFolder($3); }
	| OPEN FOLDER NAME TF    { client.openFolder($3, $4); }
	| LIST  	             { client.listContent(); }
	| LIST INTEGER           { client.listContent("", $2); }
	| LIST INTEGER INTEGER   { client.listContent(toString($2), $3); }
	| LIST NAME              { client.listContent($2, 1); }
	| LIST NAME	INTEGER      { client.listContent($2, $3); }
	| LIST VOLUME            { client.listVolume(); }
	| LIST VOLUME TF         { client.listVolume("", $3); }
	| LIST VOLUME NAME       { client.listVolume($3); }
	| LIST VOLUME NAME TF    { client.listVolume($3, $4); }
	| LIST VOLUME INTEGER    { client.listVolume(toString($3)); }
	| LIST VOLUME INTEGER TF { client.listVolume(toString($3), $4); }
    | LIST FOLDER            { client.listFolder(); }
	| EXPOSE EXPROTO         { client.expose($2); }
	| INFO                   { client.info(); }
	| CURR                   { client.current(); }
	| DESTROY 		         { client.destroy(); }
	| DESTROY TF             { client.destroy($2); }
	| CANCEL		         { client.cancel(); }
    | PROV                   { client.provision(); }
    | PROV INTEGER           { client.provision("", $2); }
	| PROV NAME              { client.provision($2); }
	| PROV NAME INTEGER      { client.provision($2, $3); }
	| PROV URL               { client.provision($2); }
	| PROV URL INTEGER       { client.provision($2, $3); }
	| UPDATE TSTART NAME     { client.adjustSchedule($3, ""); }
	| UPDATE TSTOP NAME      { client.adjustSchedule("", $3); }
	| UPDATE NAME NAME       { client.adjustSchedule($2, $3); }
	| MSET                   { client.getMetaData(); }
	| MSET NAME EQ INTEGER   { client.setMetaData($2, toString($4)); }
	| MSET NAME EQ NAME      { client.setMetaData($2, $4); }
	| PSET NAME EQ INTEGER   { client.setSysMD($2, toString($4)); }
	| PSET NAME EQ NAME      { client.setSysMD($2, $4);}
	| SET NAME EQ INTEGER    { client.setProperty($2, toString($4)); }
	| SET NAME EQ NAME       { client.setProperty($2, $4); }
	| SYNC                   { client.syncWithFS(); }
    | UP                     { client.parent(); }
    | EXPORT NAME NAME INTEGER INTEGER  	     { client.exportContent($2,$3,$4,$5); }
    | EXPORT NAME NAME INTEGER INTEGER TF        { client.exportContentWithCost($2,$3,$4,$5,$6);}
    | CACHE NAME NAME		 { client.cache($2,$3); }
	| TIMER TSTART           { client.timer(true); }
	| TIMER TSTOP            { client.timer(false); }
	| HASHFOLDER  NAME	     { client.findContent($2);}
	| HASHFOLDERNAME  NAME	 { client.hashFolderName($2);}
	| SLEEP INTEGER			 { client.mySleep($2);}
	| ACCESSCOUNT NAME          { client.getAccessCount($2);}
	| ADDACCESSCOUNT NAME INTEGER  { client.addAccessCount($2,$3);}
	| LISTMISSED INTEGER     { client.getMissedList($2); }
	| LISTHOT INTEGER        { client.getHotList($2); }
	| SETCACHEWINDOW         { client.setAccessThreshold(); }
	| CACHEWINDOW            { client.getAccessThreshold(); }
	| STOREDISTANCE NAME NAME     { client.cacheDistance($2,$3); }
	| STOREDISTANCE NAME     { client.cacheDistanceList($2); }
	| LOCALFN NAME NAME      { client.nameOfLocal($2,$3); }
	| SYSCOMMAND             { client.sysCmd(yytext); }
	| error                  {}
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

