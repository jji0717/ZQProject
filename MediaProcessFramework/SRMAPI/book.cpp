
#include "book.h"
#include "metaresource.h"
#include <algorithm>
#include "mpfexception.h"

using ZQ::MPF::SRM::MetaRecord;
using ZQ::MPF::SRM::MetaResource;
using ZQ::MPF::SRM::ResourceManager;
using ZQ::MPF::utils::NodePath;
using ZQ::MPF::SRMException;
using ZQ::MPF::utils::UniqueId;

bool DSTimePeriod::load(string timeperiodentry)
{
	try
	{
		period_id = ZQ::MPF::utils::NodePath::getPureName(timeperiodentry);

		MetaRecord timePeriod(timeperiodentry.c_str(), PM_PROP_READ_ONLY);

		start_time	= timePeriod.get(DB_KEY_START_TIME);
		end_time	= timePeriod.get(DB_KEY_END_TIME);
		number		= timePeriod.get(DB_KEY_NUMBER);

		return ((ERROR_NUMBER != start_time)&&
			(ERROR_NUMBER != end_time)&&
			(ERROR_NUMBER != number));
	}
	catch(SRMException exp)
	{
		//error log - can not open period entry
		return false;
	}
}

bool DSTimePeriod::save(string timeperiodentry)
{
	try
	{
		MetaRecord timePeriod(timeperiodentry.c_str());

		if (!timePeriod.set(DB_KEY_START_TIME, start_time))
			return false;

		if (!timePeriod.set(DB_KEY_END_TIME, end_time))
			return false;
		
		if (!timePeriod.set(DB_KEY_NUMBER, number))
			return false;

		return true;
	}
	catch(SRMException exp)
	{
		//error log - can not open period entry
		return false;
	}
}

bool DSTimePeriod::operator==(const DSTimePeriod& period) const
{
	return (start_time==period.start_time)&&(end_time==period.end_time)&&(number==period.number);
}

bool DSSubResource::operator==(const DSSubResource& subres) const
{
	return sub_res_id == subres.sub_res_id;
}

bool DSSubResource::load(string subresentry)
{
	try
	{
		sub_res_id = ZQ::MPF::utils::NodePath::getPureName(subresentry);

		MetaRecord subRes(subresentry.c_str(), PM_PROP_READ_ONLY);

		max_limit	= subRes.get(DB_KEY_MAX_LIMIT);
		if (ERROR_NUMBER == max_limit)
			return false;

		period_list.clear();

		size_t count;
		char** pstrEntry = subRes.listChildren(count);
		bool result = true;
		for (int i = 0; i < count; ++i)
		{
			DSTimePeriod tp;
			if (!tp.load(pstrEntry[i]))
				result = false;

			period_list.push_back(tp);
		}

		if (pstrEntry)
			subRes.deleteList(pstrEntry, count);

		return result;
	}
	catch(SRMException exp)
	{
		//error log - can not open period entry
		return false;
	}
}

bool DSSubResource::save(string subresentry)
{
	try
	{
		MetaRecord root(DB_ROOT DB_ROOT);
		root.remove(subresentry.c_str());

		MetaRecord subRes(subresentry.c_str());

		//not save max limit here

		bool result = true;
		for (int i = 0; i < period_list.size(); ++i)
		{
			char periodEntry[MAX_DB_ENTRY_LEN] = {0};
			_snprintf(periodEntry, MAX_DB_ENTRY_LEN-1, "%s%s%s",
				subresentry.c_str(), DB_SPEC, period_list[i].period_id);

			if (!period_list[i].save(periodEntry))
				result = false;
		}

		return result;
	}
	catch(SRMException exp)
	{
		//error log - can not open period entry
		return false;
	}
}

bool DSResource::operator==(const DSResource& subres) const
{
	return res_id == subres.res_id;
}

bool DSResource::load(string restype, string resid)
{
	try
	{
		res_id = resid;

		char bookrecord[MAX_DB_ENTRY_LEN] = {0};
		_snprintf(bookrecord, MAX_DB_ENTRY_LEN, "%s%s%s%s%s%s%s",
			DB_RESOURCE_ROOT, DB_SPEC, restype.c_str(), DB_SPEC, resid.c_str(), DB_SPEC, DB_ENTRY_BOOK);
		MetaRecord book(bookrecord, PM_PROP_READ_ONLY);

		subres_list.clear();

		size_t count;
		char** pstrEntry = book.listChildren(count);
		bool result = true;
		for (int i = 0; i < count; ++i)
		{
			DSSubResource sr;
			if (!sr.load(pstrEntry[i]))
				result = false;

			subres_list.push_back(sr);
		}

		if (pstrEntry)
			book.deleteList(pstrEntry, count);

		return result;
	}
	catch(SRMException exp)
	{
		//error log - can not open period entry
		return false;
	}
}

bool DSResource::save(string restype, string resid)
{
	try
	{
		char bookrecord[MAX_DB_ENTRY_LEN] = {0};
		_snprintf(bookrecord, MAX_DB_ENTRY_LEN, "%s%s%s%s%s%s%s",
			DB_RESOURCE_ROOT, DB_SPEC, restype.c_str(), DB_SPEC, resid.c_str(), DB_SPEC, DB_ENTRY_BOOK);

		MetaRecord root(DB_ROOT DB_ROOT);
		root.remove(bookrecord);

		bool result = true;
		for (int i = 0; i < subres_list.size(); ++i)
		{
			char subresEntry[MAX_DB_ENTRY_LEN] = {0};
			_snprintf(subresEntry, MAX_DB_ENTRY_LEN-1, "%s%s%s%s%s",
				bookrecord, DB_SPEC, DB_ENTRY_BOOK, DB_SPEC, subres_list[i].sub_res_id.c_str());

			if (!subres_list[i].save(subresEntry))
				result = false;
		}

		return result;
	}
	catch(SRMException exp)
	{
		//error log - can not open period entry
		return false;
	}
}

bool DSResourceType::load(std::string restype)
{
	try
	{
		char typerecord[MAX_DB_ENTRY_LEN] = {0};
		_snprintf(typerecord, MAX_DB_ENTRY_LEN, "%s%s%s",
			DB_RESOURCE_ROOT, DB_SPEC, restype.c_str());
		MetaRecord typeEntry(typerecord, PM_PROP_READ_ONLY);

		res_list.clear();

		size_t count;
		char** pstrEntry = typeEntry.listChildren(count);
		bool result = true;
		for (int i = 0; i < count; ++i)
		{
			DSResource rs;
			if (!rs.load(restype, NodePath::getPureName(pstrEntry[i])))
				result = false;

			res_list.push_back(rs);
		}

		if (pstrEntry)
			typeEntry.deleteList(pstrEntry, count);

		return result;
	}
	catch(SRMException exp)
	{
		//error log - can not open period entry
		return false;
	}
}

bool DSResourceType::save(std::string restype)
{
	try
	{
		bool result = true;
		for (int i = 0; i < res_list.size(); ++i)
		{
			if (!res_list[i].save(restype, res_list[i].res_id))
				result = false;
		}

		return result;
	}
	catch(SRMException exp)
	{
		//error log - can not open period entry
		return false;
	}
}

bool BookTime::operator<(const BookTime& bt) const
{
	return time < bt.time;
}


inline bool plusPoint(std::list<BookTime>::iterator& itor, size_t number, size_t maxlimit)
{
	if (itor->number + number > maxlimit)
		return false;

	itor->number += number;
		return true;
}

inline bool mulPoint(std::list<BookTime>::iterator& itor, size_t number)
{
	if (itor->number < number)
		return false;

	itor->number -= number;
	return true;
}

bool TimeLine::insertPoint(TimeItor& itor, size_t time, size_t number, bool addPrev)
{
	BookTime bt = {time, number};

	if (itor->time == time)
		return plusPoint(itor, number, m_maxLimit);

	bool bAdd = false;
	if (itor != m_pointList.begin())
	{
		--itor;
		if (itor->time == time)
		{
			if (!plusPoint(itor, number, m_maxLimit))
			{
				++itor;
				return false;
			}
			bAdd = true;
		}
		if (addPrev)
			bt.number += itor->number;
		else
			bt.number = itor->number - number;

		++itor;
	}

	if (itor != m_pointList.end())
	{
		++itor;
		if (itor->time == time)
		{
			if (!plusPoint(itor, number, m_maxLimit))
			{
				--itor;
				return false;
			}
			bAdd = true;
		}
		--itor;

	}


	if (!bAdd)
		m_pointList.insert(itor, bt);

	return true;
}

void TimeLine::merge()
{
	for (std::list<BookTime>::iterator itor = m_pointList.begin(); itor != m_pointList.end();)
	{
		size_t prev = itor->number;
		std::list<BookTime>::iterator temp = itor;
		++itor;
		bool bRollBack = false;
		while(itor->number == prev)
		{
			itor->number = size_t(-1);
			++itor;
			bRollBack = true;
		}
		if (bRollBack)
			itor = temp;
	}

	for (itor = m_pointList.begin(); itor != m_pointList.end();)
	{
		std::list<BookTime>::iterator temp = itor;
		++itor;

		if (temp->number == size_t(-1))
			m_pointList.erase(temp);
	}
}

TimeLine::TimeLine(DSSubResource& resource)
:m_maxLimit(resource.max_limit)
{
	m_pointList.clear();

	for (DSSubResource::PeriodItor itor = resource.period_list.begin();
	itor != resource.period_list.end(); ++itor)
	{
		DSTimePeriod tp;
		tp.start_time	= itor->start_time;
		tp.end_time		= itor->end_time;
		tp.number		= itor->number;
		if (!addPeriod(tp))
			throw SRMException("Can not generate time line");
	}
}

bool TimeLine::addPeriod(DSTimePeriod& period)
{
	if (period.number > m_maxLimit)
	{
		//error log here
		return false;
	}

	if (period.start_time >= period.end_time)
	{
		//error log here
		return false;
	}

	BookTime btBegin	= {period.start_time, period.number};
	BookTime btEnd		= {period.end_time, 0};

	TimeItor itorBegin	= std::upper_bound(m_pointList.begin(), m_pointList.end(), btBegin);
	TimeItor itorEnd	= std::lower_bound(itorBegin, m_pointList.end(), btEnd);

	if (itorBegin != m_pointList.begin())
	{
		--itorBegin;
		size_t temp = itorBegin->number + period.number;
		if (itorBegin->number + period.number > m_maxLimit)
			return false;
		++itorBegin;
	}

	bool bAddPointer = true;
	for (TimeItor itor = itorBegin; itor != itorEnd; ++itor)
	{
		if (!plusPoint(itor, period.number, m_maxLimit))
		{
			for (TimeItor itorMul = itorBegin; itorMul != itor; ++itorMul)
				mulPoint(itor, period.number);

			bAddPointer = false;
		}
	}

	if (bAddPointer)
	{
		insertPoint(itorBegin, period.start_time, period.number, true);
		insertPoint(itorEnd, period.end_time, period.number, false);
	}

	return bAddPointer;
}

bool TimeLine::verifyPeriod(DSTimePeriod& period)
{
	if (period.number > m_maxLimit)
	{
		//error log here
		return false;
	}

	if (period.start_time >= period.end_time)
	{
		//error log here
		return false;
	}

	BookTime btBegin	= {period.start_time, period.number};
	BookTime btEnd		= {period.end_time, 0};

	TimeItor itorBegin	= std::upper_bound(m_pointList.begin(), m_pointList.end(), btBegin);
	TimeItor itorEnd	= std::lower_bound(itorBegin, m_pointList.end(), btEnd);

	if (itorBegin != m_pointList.begin())
	{
		if ((--itorBegin)->time != period.start_time)
			++itorBegin;
	}

	for (TimeItor itor = itorBegin; itor != itorEnd; ++itor)
	{
		if (itor->number + period.number > m_maxLimit)
			return false;
	}

	return true;
}

size_t TimeLine::getNumber(size_t bookTime) const
{
	BookTime bt = {bookTime, 0};

	std::list<BookTime>::const_iterator itor = std::upper_bound(m_pointList.begin(), m_pointList.end(), bt);

	if (m_pointList.end() == itor)
		return 0;

	return itor->number;
}

void TimeLine::clear()
{
	m_pointList.clear();
}

#ifdef _TEST
void TimeLine::test()
{

	printf("List Point\n");
	for (std::list<BookTime>::iterator itor = m_pointList.begin();
	itor != m_pointList.end(); ++itor)
	{
		printf("Time: %lu, Number: %lu\n\n", itor->time, itor->number);
	}
}
#endif


ResourceBooker::ResourceBooker(string restype, string resid)
:m_stat(BOOK_STAT_INIT), m_restype(restype), m_resid(resid)
{
	if (!m_res.load(restype, resid))
		throw SRMException("Can not load resource book information");
}

ResourceBooker::~ResourceBooker()
{
	if (!m_res.save(m_restype, m_resid))
		throw SRMException("Can not load resource book information");
}

bool ResourceBooker::isStartAction(size_t& time, ActionList& al)
{
	if (m_stat != BOOK_STAT_INIT)
		return false;

	bool result = false;
	string periodId;
	for (DSResource::SubResItor itorSubRes = m_res.subres_list.begin();
	itorSubRes != m_res.subres_list.end(); ++itorSubRes)
	{
		for (DSSubResource::PeriodItor itorPeriod = itorSubRes->period_list.begin();
		itorPeriod != itorSubRes->period_list.end(); ++itorPeriod)
		{
			if (result)
			{
				if (periodId == itorPeriod->period_id)
				{
					ActionInfo ai;
					ai.sub_res	= itorSubRes->sub_res_id;
					ai.number	= itorPeriod->number;
					al.push_back(ai);

					break;
				}
			}
			else
			{
				time_t curtime;
				::time(&curtime);

				if (curtime >= itorPeriod->start_time)
				{
					time = itorPeriod->start_time;
					result = true;

					periodId = itorPeriod->period_id;

					ActionInfo ai;
					ai.sub_res	= itorSubRes->sub_res_id;
					ai.number	= itorPeriod->number;
					al.push_back(ai);

					break;
				}
			}
		}
	}

	return result;
}

bool ResourceBooker::isEndAction(size_t& time, ActionList& al)
{
	if (m_stat != BOOK_STAT_RUN)
		return false;

	bool result = false;
	string periodId;
	for (DSResource::SubResItor itorSubRes = m_res.subres_list.begin();
	itorSubRes != m_res.subres_list.end(); ++itorSubRes)
	{
		for (DSSubResource::PeriodItor itorPeriod = itorSubRes->period_list.begin();
		itorPeriod != itorSubRes->period_list.end(); ++itorPeriod)
		{
			if (result)
			{
				if (periodId == itorPeriod->period_id)
				{
					ActionInfo ai;
					ai.sub_res	= itorSubRes->sub_res_id;
					ai.number	= itorPeriod->number;
					al.push_back(ai);

					break;
				}
			}
			else
			{
				time_t curtime;
				::time(&curtime);

				if (curtime >= itorPeriod->end_time)
				{
					time = itorPeriod->end_time;
					result = true;

					periodId = itorPeriod->period_id;

					ActionInfo ai;
					ai.sub_res	= itorSubRes->sub_res_id;
					ai.number	= itorPeriod->number;
					al.push_back(ai);

					break;
				}
			}
		}
	}

	return result;
}

bool ResourceBooker::verify(size_t startTime, size_t endTime, ActionList& al)
{
	for (ActionItor itorAction = al.begin(); itorAction != al.end(); ++itorAction)
	{
		DSSubResource subres;
		subres.sub_res_id = itorAction->sub_res;

		DSResource::SubResItor itorSubRes = std::find(m_res.subres_list.begin(), m_res.subres_list.end(), subres);
		if (m_res.subres_list.end() == itorSubRes)
			return false;

		TimeLine tl(*itorSubRes);
		DSTimePeriod timePeriod;
		timePeriod.start_time	= startTime;
		timePeriod.end_time		= endTime;
		timePeriod.number		= itorAction->number;

		if (!tl.verifyPeriod(timePeriod))
			return false;
	}
	return true;
}

bool ResourceBooker::book(size_t startTime, size_t endTime, ActionList& al)
{
	for (ActionItor itorAction = al.begin(); itorAction != al.end(); ++itorAction)
	{
		DSSubResource subres;
		subres.sub_res_id = itorAction->sub_res;

		DSResource::SubResItor itorSubRes = std::find(m_res.subres_list.begin(), m_res.subres_list.end(), subres);
		if (m_res.subres_list.end() == itorSubRes)
			return false;


		DSTimePeriod timePeriod;
		UniqueId unid;
		timePeriod.period_id	= (const char*)(unid);
		timePeriod.start_time	= startTime;
		timePeriod.end_time		= endTime;
		timePeriod.number		= itorAction->number;

		itorSubRes->period_list.push_back(timePeriod);
	}
	return true;
}

bool ResourceBooker::remove(size_t startTime, size_t endTime, ActionList& al)
{
	DSTimePeriod period;
	period.start_time	= startTime;
	period.end_time		= endTime;

	for (DSResource::SubResItor itor = m_res.subres_list.begin();
	itor != m_res.subres_list.end(); ++itor)
	{
		bool bFind = false;
		for (ActionItor itorSubRes = al.begin(); itorSubRes != al.end(); ++itorSubRes)
		{
			DSTimePeriod period;
			period.start_time	= startTime;
			period.end_time		= endTime;
			period.number		= itorSubRes->number;

			DSSubResource::PeriodItor itorFind = std::find(itor->period_list.begin(), itor->period_list.end(), period);
			bFind = (itor->period_list.end() != itorFind);
			break;
		}

		if (!bFind)
			return false;
	}

	for (itor = m_res.subres_list.begin();
	itor != m_res.subres_list.end(); ++itor)
	{
		for (ActionItor itorSubRes = al.begin(); itorSubRes != al.end(); ++itorSubRes)
		{
			DSTimePeriod period;
			period.start_time	= startTime;
			period.end_time		= endTime;
			period.number		= itorSubRes->number;

			DSSubResource::PeriodItor itorFind = std::find(itor->period_list.begin(), itor->period_list.end(), period);
			itor->period_list.erase(itorFind);
			break;
		}
	}
	return true;
}

bool ResourceBooker::operator==(string resid) const
{
	return m_resid == resid;
}

bool Booker::OnStartAction(size_t time, string resId, const ActionList& subResList)
{
	return true;
}

void Booker::OnEndAction(size_t time, string resId, const ActionList& subResList)
{
}

Booker::ResBookItor Booker::find(string resid)
{
	for (ResBookItor itor = m_bookList.begin(); itor != m_bookList.end(); ++itor)
	{
		if (*itor == resid)
			return itor;
	}

	return m_bookList.end();
}

Booker::Booker(string restype)
:m_restype(restype), m_bQuit(false)
{
	DSResourceType rt;
	if (!rt.load(restype))
		throw SRMException("Can not load book information");

	m_bookList.clear();

	for (DSResourceType::ResItor itor = rt.res_list.begin(); itor != rt.res_list.end(); ++itor)
	{
		ResourceBooker rb(restype, itor->res_id);
		m_bookList.push_back(rb);
	}
}

Booker::~Booker()
{
	m_bQuit = true;

	waitHandle(2000);
	exit();
	/*
	if (!rt.save(m_restype))
		throw SRMException("Can not save book information");
	*/
}

int Booker::run()
{
	while (!m_bQuit)
	{
		for (ResBookItor itor = m_bookList.begin(); itor != m_bookList.end(); ++itor)
		{
			size_t tempTime;
			ActionList al;
			if (itor->isStartAction(tempTime, al))
			{
				OnStartAction(tempTime, itor->getResId(), al);
			}
			else if (itor->isEndAction(tempTime, al))
			{
				OnEndAction(tempTime, itor->getResId(), al);
			}
		}
	}
	return 0;
}

Booker::ResList Booker::getResList()
{
	Booker::ResList result;

	for (ResBookItor itor = m_bookList.begin(); itor != m_bookList.end(); ++itor)
	{
		result.push_back(itor->getResId());
	}

	return result;
}

bool Booker::verify(size_t beginTime, size_t endTime, ActionList& subResList, ResList& reslist)
{
	ResList temp;

	bool result = false;
	for (ResItor itor = reslist.begin(); itor != reslist.end(); ++itor)
	{
		ResBookItor itorFind = find(*itor);
		if (itorFind == m_bookList.end())
			continue;

		if (itorFind->verify(beginTime, endTime, subResList))
		{
			result = true;
			temp.push_back(itorFind->getResId());
		}
	}

	reslist = temp;
	return result;
}

bool Booker::book(size_t beginTime, size_t endTime, string resId, ActionList& subResList)
{
	ResBookItor itorFind = find(resId);
	if (itorFind == m_bookList.end())
		return false;

	return itorFind->book(beginTime, endTime, subResList);
}

bool Booker::remove(size_t beginTime, size_t endTime, string resId, ActionList& subResList)
{
	ResBookItor itorFind = find(resId);
	if (itorFind == m_bookList.end())
		return false;

	return itorFind->remove(beginTime, endTime, subResList);
}



