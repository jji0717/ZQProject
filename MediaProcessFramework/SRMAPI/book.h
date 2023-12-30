
#ifndef _ZQ_BOOK_H_
#define _ZQ_BOOK_H_

#include "nativethread.h"

#pragma warning(disable:4786)

#include <map>
#include <list>
#include <vector>
#include <string>

using std::string;

#define DB_KEY_START_TIME	"StartTime"
#define DB_KEY_END_TIME		"EndTime"
#define DB_KEY_NUMBER		"Number"
#define DB_ENTRY_PERIOD		"Period_"
#define ERROR_NUMBER		size_t(-1)
#define DB_KEY_MAX_LIMIT	"MaxLimit"
#define DB_ENTRY_BOOK		"Book"

/*
 * Data structure:
 * +--Resource Manager(resource type[res_type])
 *   |
 *   +--Resource(resource id[res_id])
 *     |
 *	   +--Hidden Book Record(book node[DB_ENTRY_BOOK])
 *     | |
 *     | +--Sub Resource(sub_res)
 *     | | |
 *     | | |--Sub Resource Number Max(max_limit)
 *     | | |
 *     | | +--Time Period("Period_%d")
 *     | | | |
 *     | | | |--Start Time(start_time)
 *     | | | |--End Time(end_time)
 *     | | | |--Number(number)
 *     | | |
 *     | | +--Time Period("Period_%d")
 *     | | | |
 *     | | | |--Start Time(start_time)
 *     | | | |--End Time(end_time)
 *     | | | |--Number(number)
 *     | | |
 *     | | |--...
 *     | |
 *     | |
 *     | +--...
 *     |
 *     |--Sub Resource
 *     |--Sub Resource
 *     |--...
 */
struct DSTimePeriod
{
	size_t		start_time;
	size_t		end_time;
	size_t		number;
	string		period_id;

	bool load(string timeperiodentry);
	bool save(string timeperiodentry);

	bool operator==(const DSTimePeriod& period) const;
};

struct DSSubResource
{
	typedef std::vector<DSTimePeriod>	PeriodList;
	typedef PeriodList::iterator		PeriodItor;

	string		sub_res_id;
	size_t		max_limit;
	PeriodList	period_list;

	bool operator==(const DSSubResource& subres) const;
	
	bool load(string subresentry);
	bool save(string subresentry);
};

struct DSResource
{
	typedef std::vector<DSSubResource>	SubResList;
	typedef SubResList::iterator		SubResItor;

	string		res_id;
	SubResList	subres_list;

	bool operator==(const DSResource& subres) const;

	bool load(string restype, string resid);
	bool save(string restype, string resid);
};

struct DSResourceType
{
	typedef std::vector<DSResource>	ResList;
	typedef ResList::iterator		ResItor;

	ResList	res_list;

	bool load(std::string restype);
	bool save(std::string restype);
};


struct BookTime
{
	size_t					time;
	size_t					number;

	bool operator<(const BookTime& bt) const;
};

/*
 * Time Line of Book:
 * this is a time line of book which sorted by time form small to large
 * there are some special points in the line, all the points with a book number.
 * for example:
 * there are four time period for book
 * number 4 for 1:00 to 2:00
 * number 2 for 1:30 to 2:30
 * number 2 for 2:10 to 2:20
 * number 3 for 2:40 to 3:00
 * there are 8 points in the time line:
 * 1:00(4), 1:30(6), 2:00(2), 2:10(4), 2:20(2), 2:30(0), 2:40(3), 3:00(0)
 *
 * the points should merge if they are the same time.
 * for example:
 * there are two time points are the same: 2:10(3), 2:10(4)
 * they must be merged and be only one points: 2:10(7)
 */
struct TimeLine
{
public:
	typedef std::list<BookTime>	TimeList;
	typedef TimeList::iterator	TimeItor;

private:
	size_t		m_maxLimit;
	TimeList	m_pointList;

	bool insertPoint(TimeItor& itor, size_t time, size_t number, bool addPrev);

	void merge();

public:
	///constructor
	///@param maxNumber - number limit
	TimeLine(DSSubResource& resource);

	///add book period into time line
	///@param beginTime - begin time of the book period
	///@param endTime - end time of the book period
	///@param number - book number
	///@return true if ok
	bool addPeriod(DSTimePeriod& period);

	///verify period with time line
	///@param beginTime - begin time of the book period
	///@param endTime - end time of the book period
	///@param number - book number
	///@return true if the period can add into time line
	bool verifyPeriod(DSTimePeriod& period);

	///remove book period from time line
	///@param beginTime - begin time of the book period
	///@param endTime - end time of the book period
	///@param number - book number
	///@return true if ok
	//bool removePeriod(const DSTimePeriod& period);

	///get number in a time
	///@param bookTime - booked time
	///@return booked number
	size_t getNumber(size_t bookTime) const;

	//clear
	void clear();

#ifdef _TEST
	void test();
#endif
};

#define BOOK_STAT_INIT	0
#define BOOK_STAT_RUN	1
#define BOOK_STAT_END	2

struct ActionInfo
{
	string	sub_res;
	size_t	number;
};

typedef std::vector<ActionInfo>		ActionList;
typedef ActionList::iterator		ActionItor;
//typedef std::vector<DSTimePeriod>	PeriodList;

class ResourceBooker
{
private:
	DSResource	m_res;
	size_t		m_stat;

	string		m_restype;
	string		m_resid;

public:
	ResourceBooker(string restype, string resid);
	~ResourceBooker();

	bool isStartAction(size_t& time, ActionList& al);
	bool isEndAction(size_t& time, ActionList& al);

	bool verify(size_t startTime, size_t endTime, ActionList& al);
	bool book(size_t startTime, size_t endTime, ActionList& al);

	bool remove(size_t startTime, size_t endTime, ActionList& al);

	bool operator==(string resid) const;
	
	string getResId() const
	{ return m_resid; }
};

class Booker : ZQ::common::NativeThread
{
public:
	typedef std::vector<ResourceBooker>	ResBookList;
	typedef ResBookList::iterator		ResBookItor;

	typedef std::list<string>			ResList;
	typedef ResList::iterator			ResItor;

private:
	ResBookList		m_bookList;
	string			m_restype;
	bool			m_bQuit;

protected:
	///call back function which called at the start book time
	///@param beginTime - begin time of book period
	///@param endTime - end time of book period
	///@param strResource - resource id string
	///@return the book period will be remove if return true, else the false
	virtual bool OnStartAction(size_t time, string resId, const ActionList& subResList);

	virtual void OnEndAction(size_t time, string resId, const ActionList& subResList);

	ResBookItor find(string resid);

	int run();
public:
	Booker(string restype);
	virtual ~Booker();

	ResList getResList();

	///verify that the resource can be allocated
	///@param beginTime - begin time of book period
	///@param endTime - end time of book period
	///@param subResList - sub resource number of a resource
	///@return resource id list
	bool verify(size_t beginTime, size_t endTime, ActionList& subResList, ResList& reslist);

	///book a resource 
	///@param beginTime - begin time of book period
	///@param endTime - end time of book period
	///@param strResource - resource id string which in the list of return of verify function
	///@param subResList - sub resource number of a resource
	///@return ture if ok
	bool book(size_t beginTime, size_t endTime, string resId, ActionList& subResList);

	///remove a resource book
	///@param beginTime - begin time of book period
	///@param endTime - end time of book period
	///@param strResource - resource id string which added by book function
	///@param subResList - sub resource number of a resource
	///@return ture if ok
	bool remove(size_t beginTime, size_t endTime, string resId, ActionList& subResList);
};

#endif
