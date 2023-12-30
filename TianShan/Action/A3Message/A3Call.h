#include "RuleEngine.h"

using namespace ZQ::common;
/*
class A3Call : public Action
{
public:
	A3Call(RuleEngine& engine, ZQ::common::Log& log) : Action(engine, "A3Call"), _log(log) {}

public:

	virtual Action& operator() (Context& ctx, const Properties& input, Properties& output);

private:
	ZQ::common::Log& _log;
};
*/
class A3GetContentInfo : public Action
{
public:
	A3GetContentInfo(RuleEngine& engine, ZQ::common::Log& log) : Action(engine, "A3GetContentInfo"), _log(log) {}

public:

	virtual Action& operator() (Context& ctx, const Properties& input, Properties& output);

private:
	ZQ::common::Log& _log;
};

class A3ExposeContent : public Action
{
public:
	A3ExposeContent(RuleEngine& engine, ZQ::common::Log& log) : Action(engine, "A3ExposeContent"), _log(log) {}

public:

	virtual Action& operator() (Context& ctx, const Properties& input, Properties& output);

private:
	ZQ::common::Log& _log;
};

class A3TransferContent : public Action
{
public:
	A3TransferContent(RuleEngine& engine, ZQ::common::Log& log) : Action(engine, "A3TransferContent"), _log(log) {}

public:

	virtual Action& operator() (Context& ctx, const Properties& input, Properties& output);

private:
	ZQ::common::Log& _log;
};

class A3GetVolumeInfo : public Action
{
public:
	A3GetVolumeInfo(RuleEngine& engine, ZQ::common::Log& log) : Action(engine, "A3GetVolumeInfo"), _log(log) {}

public:

	virtual Action& operator() (Context& ctx, const Properties& input, Properties& output);

private:
	ZQ::common::Log& _log;
};

class A3DeleteContent : public Action
{
public:
	A3DeleteContent(RuleEngine& engine, ZQ::common::Log& log) : Action(engine, "A3DeleteContent"), _log(log) {}

public:

	virtual Action& operator() (Context& ctx, const Properties& input, Properties& output);

private:
	ZQ::common::Log& _log;
};

class A3CancelTransfer : public Action
{
public:
	A3CancelTransfer(RuleEngine& engine, ZQ::common::Log& log) : Action(engine, "A3CancelTransfer"), _log(log) {}

public:

	virtual Action& operator() (Context& ctx, const Properties& input, Properties& output);

private:
	ZQ::common::Log& _log;
};

class A3GetTransferStatus : public Action
{
public:
	A3GetTransferStatus(RuleEngine& engine, ZQ::common::Log& log) : Action(engine, "A3GetTransferStatus"), _log(log) {}

public:

	virtual Action& operator() (Context& ctx, const Properties& input, Properties& output);

private:
	ZQ::common::Log& _log;
};

class A3GetContentChecksum : public Action
{
public:
	A3GetContentChecksum(RuleEngine& engine, ZQ::common::Log& log) : Action(engine, "A3GetContentChecksum"), _log(log) {}

public:

	virtual Action& operator() (Context& ctx, const Properties& input, Properties& output);

private:
	ZQ::common::Log& _log;
};

