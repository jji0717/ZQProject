%{

#include <sstream>
#include "SSAdmin.h"

extern int yylex(void);
extern void yyerror(char*);
extern char* yytext;
extern bool isEOF;

SSAdmin admin;

extern std::string toString(int);

#ifdef ZQ_OS_MSWIN
#define CLS "cls"
#else
#define CLS "clear"
#endif

%}

%union {
	int val;
	float frac;
	char* str;
};

%token HELP EXIT CLEAR
%token CONNECT DISCON 
%token LIST CREATE PUSH PLAY PAUSE RESUME SEEK SEEKITEM SPEED RATE ERASE
%token DESTROY SELECT ALIAS CURR INFO MACADDR VERBOSE

%token <val> STREAMER ITEM ALL
%token <val> INTEGER TF NL POS
%token <frac> FRAC

%token <str> STR ADDR

%type <val> cmd content seekargs createargs

%%

prog: NL      { return (0); } 
	| cmd NL  { return (0); }
	;
	
cmd:  HELP { admin.help(); }
	| EXIT { admin.exit(); }
	| CONNECT STR { admin.connect($2); } 
	| DISCON { admin.close(); }
	| VERBOSE TF { admin.verbose($2); }
	| LIST content { $$=$2; }
	| CREATE createargs { $$=$2; }
	| PUSH STR INTEGER { admin.pushItem($2, $3); }
	| PLAY { admin.play(); }
	| PLAY INTEGER { admin.play($2); }
	| PLAY FRAC INTEGER POS { admin.playEx($2, $3, $4); } 
	| PAUSE { admin.pauseEx(); }
	| RESUME { admin.resume(); }
	| SEEK INTEGER { admin.seek($2); } 
	| SEEK INTEGER POS { admin.seek($2, $3); }
	| SEEKITEM seekargs { $$=$2; }
	| SPEED FRAC { admin.setSpeed($2); }
	| RATE INTEGER { admin.setRate($2); }
	| ERASE INTEGER { admin.erase($2); }
	| DESTROY { admin.destroy(); }
	| DESTROY ALL { admin.destroyAll(); }
	| SELECT STR { admin.select($2); }
	| SELECT INTEGER { admin.select(toString($2)); }
	| ALIAS STR STR { admin.alias($2, $3); }
	| CURR { admin.current(); }
	| INFO { admin.info(); }
	| MACADDR STR { admin.setDestMac($2); }
	| CLEAR { system(CLS); }
	| error {}
	;   
	
content: { admin.listPlaylist(); }
	| ITEM { admin.listItem(); }
	| STREAMER { admin.listStreamer(); }
	| error {}
	;
	
createargs: ADDR INTEGER INTEGER { admin.createPlaylist($1, $2, toString($3)); }
	| ADDR INTEGER STR { admin.createPlaylist($1, $2, $3); }
	| ADDR INTEGER { admin.createPlaylist($1, $2); }
	| error {}
	;

seekargs: INTEGER INTEGER POS FRAC { admin.seekItem($1, $2, $3, $4); } 
	| INTEGER INTEGER POS { admin.seekItem($1, $2, $3); }
	| INTEGER INTEGER { admin.seekItem($1, $2); }
	| error {}
	;
	
%% 

std::string toString(int val) {
	std::ostringstream oss; oss << val;
	return oss.str();
}

void yyerror(char* s) {
	if(!isEOF) {
		std::cerr << "bad command or invalid syntax, use \"help\" for more information. " << std::endl;
	}
}