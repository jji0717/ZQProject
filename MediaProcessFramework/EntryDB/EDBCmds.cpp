// ============================================================================================
// Copyright (c) 1997, 1998 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved. Unpublished rights reserved under the copyright laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the confidential
// technology of ZQ Interactive, Inc. Possession, use, duplication or dissemination of the
// software and media is authorized only pursuant to a valid written license from ZQ Interactive,
// Inc.
// This was copied from enterprise domain object sys, edos's copyright is belong to Hui Shao
//
// This software is furnished under a  license  and  may  be used and copied only in accordance
// with the terms of  such license and with the inclusion of the above copyright notice.  This
// software or any other copies thereof may not be provided or otherwise made available to any
// other person.  No title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and should not be
// construed as a commitment by ZQ Interactive, Inc.
// --------------------------------------------------------------------------------------------
// Author: Hui Shao
// Desc  : entry database command line util
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/EDBCmds.cpp 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/EDBCmds.cpp $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 3     4/14/05 10:13a Hui.shao
// 
// 2     4/12/05 6:03p Hui.shao
// ============================================================================================

#include "EDBCmds.h"
#include "Addins.h"

#define PROMPT "edb%% "

EDBCmds::EDBCmds()
        :bContinue(true), factory(NULL)
{
	factory = new EDBFactory(gblAddinManager);
	mEdb.pFactory = factory;
}

EDBCmds::~EDBCmds()
{
	if (factory!=NULL)
		delete factory;
	factory=NULL;
}

#define CMDMAP(_C) CMDMAP_SPEC(#_C, _C)
#define CMDMAP_SPEC(_C, _F) { if (args[0].compare(_C) ==0) { _F(); continue; }}

void EDBCmds::performs()
{
	while (bContinue)
	{
		readln();

		if (args.size()<=0)
			continue;

		CMDMAP(help);
		CMDMAP_SPEC("?", help);
		
		CMDMAP(quit);

		CMDMAP(load);
		CMDMAP_SPEC("ld",load);

		CMDMAP(edms);
		CMDMAP(connect);
		CMDMAP_SPEC("discon",disconnect);

		CMDMAP(list);
		CMDMAP_SPEC("ls", list);
		CMDMAP(chentry);
		CMDMAP_SPEC("ce", chentry);
		CMDMAP(mkentry);
		CMDMAP_SPEC("mk", mkentry);
		CMDMAP(set);
		CMDMAP(delentry);
		CMDMAP_SPEC("del", delentry);

		CMDMAP(export);
		CMDMAP_SPEC("xexp", xexport);
		CMDMAP(xexport);
		CMDMAP_SPEC("xexport", xexport);

		fprintf(stderr, "unknown command\n");
	}
}

void EDBCmds::help()
{
#define CMDH_FMT "%-12s"
	printf("commands:\n");
	printf(CMDH_FMT CMDH_FMT CMDH_FMT CMDH_FMT CMDH_FMT "\n",
		   "?","help","ld","load","connect","discon");
	printf(CMDH_FMT CMDH_FMT CMDH_FMT CMDH_FMT CMDH_FMT "\n",
		   "ls","list","ce","chentry","mk","mkentry");
	printf(CMDH_FMT CMDH_FMT CMDH_FMT CMDH_FMT CMDH_FMT "\n",
		   "del","delentry","set","edms","quit");
	printf(CMDH_FMT CMDH_FMT CMDH_FMT CMDH_FMT CMDH_FMT "\n",
		   "xexp","xexport","exp","export","");
	printf("\n");
#undef CMDH_FMT
}

#define BLANKS "\t \r\n"
bool EDBCmds::readln(bool prompt, const char* line)
{
	args.clear();

	if (prompt) printf(PROMPT);

	if (line == NULL || *line ==0x00)
	{
		static char buf[512];
		buf[0] =0x00;

		for (char*p = buf; *p!='\n' && *p!='\r'; p++)
		{
			*p = getchar();
			if (*p=='\r' || *p=='\n') p--;
		}

		*p=0x00;
		line = buf;
	}

	std::string wkline= line;
	argsln = "";
	for (int wordpos= wkline.find_first_not_of(BLANKS); wordpos>=0;)
	{
		int wordend = wkline.find_first_of(BLANKS, wordpos);
		if (wordend <=0)
		{
			args.push_back(wkline.substr(wordpos));
			break;
		}

		args.push_back(wkline.substr(wordpos, wordend-wordpos));
		wordpos = wkline.find_first_not_of(BLANKS, wordend);
	
		if (argsln.empty())
			argsln = wkline.substr(wordpos);
	}

	return true;

}

bool EDBCmds::shift(int num)
{
	int size =  args.size();
	if (size <=0)
		return false;

	if (size<=num)
	{
		args.clear();
		return true;
	}

	for (int i =0; i< size-1; i++)
		args[i] = args[i+num];

	for (i=0; i< num; i++)
		args.pop_back();

	return true;
}

#define ASSET_DBCONN if (!mEdb.isConnected()) { fprintf(stderr, "no database connected\n"); return;}

void EDBCmds::load()
{
	if (args.size()<1 || args[1].compare("-h")==0)
	{
		printf("syntax: load|ld [-h] [<path>]\n");
		return;
	}

	std::string path = args[1].empty() ? "." : args[1];

	if (factory!=NULL)
		delete factory;
	factory=NULL;

	gblAddinManager.populate(path.c_str());

	factory = new EDBFactory(gblAddinManager);
	mEdb.pFactory = factory;
}

void EDBCmds::list()
{
	if (args.size()>1 && args[1].compare("-h")==0)
	{
		printf("syntax: list|ls [-h|-l] [entry]\n");
		return;
	}

	ASSET_DBCONN;

	bool bDetail=false;
	std::string crnte = mEdb.getCurrentEntry();
	std::string e2ls = crnte;

	if (args.size()>1 && args[1].compare("-c")==0)
	{
		printf("connection: %s\n", mEdb.URL());
		return;
	}

	if (args.size()>1 && args[1].compare("-l")==0)
	{
		bDetail=true;
		shift();
	}

	if (args.size()>1 && !args[1].empty())
	{
		if (args[1][0] == LOGIC_SEPC)
			e2ls = args[1];
		else if (args[1][0] =='.' && args[1][1] =='.')
		{
			e2ls = crnte.substr(0, crnte.find_last_of(LOGIC_SEPC));
			if (e2ls.empty())
				e2ls = LOGIC_SEPS;
		}
		else e2ls = crnte + LOGIC_SEPS + args[1];
	}

	if (e2ls==crnte || mEdb.openEntry(e2ls.c_str()))
	{
		printf("%c [%s]\n", (bDetail? 'o':'+'), e2ls.c_str());

		for(int i =0; bDetail; i++)
		{
			const char* attrname = mEdb.getAttrName(i);
			if (attrname==NULL)
				break;
			printf("    (%s) %s\n", attrname, mEdb.getAttribute(attrname));
		}
		
		for (bool succ = mEdb.openFirstChild(); succ; succ=mEdb.openNextSibling())
		{
			printf("  %c [%s]\n", (bDetail? '-':'+'), mEdb.getCurrentEntry());
			for(int i =0; bDetail; i++)
			{
				const char* attrname = mEdb.getAttrName(i);
				if (attrname==NULL)
					break;
				printf("      (%s) %s\n", attrname, mEdb.getAttribute(attrname));
			}
		}
	}
	else
		fprintf(stderr, "error: could not open %s\n", e2ls.c_str());

	mEdb.openEntry(crnte.c_str());
}

void EDBCmds::mkentry()
{
	if (args.size()<1 || args[1].compare("-h")==0)
	{
		printf("syntax: mkentry|mk [-h] <entry>\n");
		return;
	}

	ASSET_DBCONN;

	std::string crnte = mEdb.getCurrentEntry();
	std::string e2mk = (args[1][0] == LOGIC_SEPC) ? args[1] : (crnte + LOGIC_SEPS + args[1]);

	if (!mEdb.openEntry(e2mk.c_str()))
	{
		if (mEdb.openEntry(e2mk.c_str(), true))
			printf("[%s] created\n", e2mk.c_str());
		else
			fprintf(stderr, "error: could not make %s\n", e2mk.c_str());
	}
	else
		fprintf(stderr, "error: [%s] already exists\n", e2mk.c_str());

	mEdb.openEntry(crnte.c_str());
}

void EDBCmds::chentry()
{
	if (args.size()<1 || args[1].compare("-h")==0)
	{
		printf("syntax: chentry|ce [-h] <entry>\n");
		return;
	}

	ASSET_DBCONN;

	std::string crnte = mEdb.getCurrentEntry();
	std::string e2ce = crnte;
	
	if (!args[1].empty())
	{
		if (args[1][0] == LOGIC_SEPC)
			e2ce = args[1];
		else if (args[1][0] =='.' && args[1][1] =='.')
		{
			e2ce = crnte.substr(0, crnte.find_last_of(LOGIC_SEPC));
			if (e2ce.empty())
				e2ce = LOGIC_SEPS;
		}
		else e2ce = crnte + LOGIC_SEPS + args[1];
	}

	if (e2ce == crnte)
		return;

	if (!mEdb.openEntry(e2ce.c_str()))
	{
		fprintf(stderr, "error: could not change current to entry %s\n", e2ce.c_str());
		mEdb.openEntry(crnte.c_str());
	}
}

void EDBCmds::delentry()
{
	bool confirmed= false;

	if (args.size()<1 || args[1].compare("-h")==0)
	{
		printf("syntax: delentry|del [-h|-y] <entry>\n");
		return;
	}

	if (args[1].compare("-y")==0)
	{
		confirmed=true;
		shift();
	}

	if (args.size()<2)
		return;

	ASSET_DBCONN;

	std::string crnte = mEdb.getCurrentEntry();
	std::string e2del = (args[1][0] == LOGIC_SEPC) ? args[1] : (crnte + LOGIC_SEPS + args[1]);

	if (!confirmed)
	{
		printf("delete entry %s? [yes|no]", e2del.c_str());
		readln(false);
		confirmed = (args[0][0] =='y' ||args[0][0] =='Y');
	}

	if (!confirmed)
		return;

	if (crnte.compare(0, e2del.length(), e2del, 0, e2del.length()) ==0)
	{
		fprintf(stderr, "could not delete currently opened entry\n");
		return;
	}

	if (!mEdb.deleteEntry(e2del.c_str()))
		fprintf(stderr, "error: could not delete entry %s\n", e2del.c_str());

	mEdb.openEntry(crnte.c_str());
}

void EDBCmds::set()
{
	if (args.size()<2 || args[1].compare("-h")==0)
	{
		printf("syntax: set [-h] <attribute>=[<value>]\n");
		return;
	}

	ASSET_DBCONN;

	int pos = argsln.find('=');
	std::string attr, val;

	if (pos<=0)
		attr = argsln;
	else
	{
		attr = argsln.substr(0, pos);
		val = argsln.substr(pos+1);
	}

	if (attr.empty() || !mEdb.setAttribute(attr.c_str(), val.c_str()))
		fprintf(stderr, "failed to set attribute %s\n", args[1].c_str());
}

void EDBCmds::quit()
{
	if (args.size()>1 && args[1].compare("-h")==0)
	{
		printf("syntax: quit\n");
		return;
	}

	printf("bye\n");
	bContinue =false;
}

void EDBCmds::connect()
{
	if (args.size()<2 || args[1].compare("-h")==0)
	{
		printf("syntax: connect <entry database URL>\n");
		return;
	}

	if (args[1].empty())
	{
		fprintf(stderr, "error: no entry database URL specified\n");
		return;
	}

	if (mEdb.connect(args[1].c_str()))
		printf("database connected\n");
	else
		fprintf(stderr, "fail to connect to database\n");
}

void EDBCmds::disconnect()
{
	if (args.size()>1 && args[1].compare("-h")==0)
	{
		printf("syntax: discon\n");
		return;
	}

	ASSET_DBCONN;

	mEdb.disconnect();
}

void EDBCmds::edms()
{
	if (args.size()>1 && args[1].compare("-h")==0)
	{
		printf("syntax: edms [-h|-l]\n");
		return;
	}
	
	bool detail = (args.size()>1 && args[1].compare("-l")==0);

	int i=0;
	const char *edmfilename=NULL;
	for (edmfilename = mEdb.pFactory->ModuleFile(i); edmfilename; edmfilename = mEdb.pFactory->ModuleFile(++i))
	{
		printf("[%s]\n", mEdb.pFactory->ModuleDBDesc(i));
		if (detail)
			printf("  URL     : %s\n  file    : %s\n  instance: %d\n",
			       mEdb.pFactory->ModuleURLHelp(i),
				   mEdb.pFactory->isModuleInternal(i) ? "<internal>" : edmfilename,
				   mEdb.pFactory->ModuleInstance(i));
	}

	if (i==0)
		fprintf(stderr, "no entry database module has been loadded, please use command 'load' first\n");
}

void EDBCmds::export()
{
	if (args.size()>1 && args[1].compare("-h")==0)
	{
		printf("syntax: export|exp [-h] <edbURL>\n");
		return;
	}
	
	ASSET_DBCONN;

	if (args[1].empty())
	{
		fprintf(stderr, "error: no entry database URL specified\n");
		return;
	}

	EDB edb2exp;

	if (!edb2exp.connect(args[1].c_str()))
	{
		fprintf(stderr, "fail to connect to database %s\n", args[1].c_str());
		return;
	}

	mEdb.export(edb2exp);

	edb2exp.disconnect();
}

void EDBCmds::xexport()
{
	if (args.size()>1 && args[1].compare("-h")==0)
	{
		printf("syntax: navigate|nav [-h|-f <filename>] <edbURL>\n");
		return;
	}

	ASSET_DBCONN;

	std::string crnte = mEdb.getCurrentEntry();
	std::string e2nav;
	std::string outfile;

	if (args.size()>1 && !args[1].empty())
	{
		if (args[1].compare("-f")==0)
		{
			shift();
			if (args.size()<2)
			{
				fprintf(stderr, "no output file specified\n");
				return;
			}

			outfile =  args[1];
			shift();
		}
	}
	
	if (args.size()>1 && !args[1].empty())
	{
		if (args[1][0] == LOGIC_SEPC)
			e2nav = args[1];
		else if (args[1][0] =='.' && args[1][1] =='.')
		{
			e2nav = crnte.substr(0, crnte.find_last_of(LOGIC_SEPC));
			if (e2nav.empty())
				e2nav = LOGIC_SEPS;
		}
		else e2nav = crnte + LOGIC_SEPS + args[1];
	}

	mEdb.xexport(e2nav.c_str(), outfile.c_str());
}

