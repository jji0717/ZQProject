
/*
 * 
 * AlertComponent.cpp
 *
 *
 * Class AlertComponent Methods
 *  
 *
 *
 *  Revision History
 *  
 *  03-10-2010 Created ( jie.zhang@schange.com)
 *  
 * 
 */

#include <stdexcept>
#include <iostream>
#include <list>
#include <stdlib.h>
#include <sstream>
#include <string>
#include "common.h"

using namespace std;

AlertComponent::AlertComponent()   // Constructor
{
	ComponentName	= "UNKNOWN";	
	TimeStamp		= "UNKNOWN";
	Severity		= "UNKNOWN";
	descr			= "UNKNOWN";
	recc			= "UNKNOWN";
}

AlertComponent::AlertComponent(const AlertComponent &copyin)   // Copy constructor to handle pass by value.
{                            
	ComponentName	= copyin.ComponentName;
	TimeStamp		= copyin.TimeStamp;
	Severity		= copyin.Severity;
	descr			= copyin.descr;
	recc			= copyin.recc;
}


AlertComponent& AlertComponent::operator=(const AlertComponent &rhs)
{
	this->ComponentName	= rhs.ComponentName;
	this->TimeStamp		= rhs.TimeStamp;
	this->Severity		= rhs.Severity;
	this->descr			= rhs.descr;
	this->recc			= rhs.recc;
	return *this;
}

int AlertComponent::operator==(const AlertComponent &rhs) const
{
   if(this->TimeStamp.compare(rhs.TimeStamp) == 0)		return 1;
   return 0;
}

//
// helper function to give a numeric value for month name
//
int getMonthNum(string locstring)
{
	int monthnum = 0;

	if (locstring.compare(0, 3, "Jan") == 0) {
		monthnum = 1;
	} else if (locstring.compare(0, 3, "Feb") == 0) {
		monthnum = 2;
	} else if (locstring.compare(0, 3, "Mar") == 0) {
		monthnum = 3;
	} else if (locstring.compare(0, 3, "Apr") == 0) {
		monthnum = 4;
	} else if (locstring.compare(0, 3, "May") == 0) {
		monthnum = 5;
	} else if (locstring.compare(0, 3, "Jun") == 0) {
		monthnum = 6;
	} else if (locstring.compare(0, 3, "Jul") == 0) {
		monthnum = 7;
	} else if (locstring.compare(0, 3, "Aug") == 0) {
		monthnum = 8;
	} else if (locstring.compare(0, 3, "Sep") == 0) {
		monthnum = 9;
	} else if (locstring.compare(0,3, "Oct") == 0) {
		monthnum = 10;
	} else if (locstring.compare(0, 3, "Nov") == 0) {
		monthnum = 11;
	} else if (locstring.compare(0, 3, "Dec") == 0) {
		monthnum = 12;
	} else {
		// error
		monthnum =  -1;
	}

	return monthnum;
}
// This function is required for built-in STL list functions like sort
// this determines via substr values less than condition and is used by sort()
// 
//
int AlertComponent::operator<(const AlertComponent &rhs) const
{
	int thismonth	= 0;
	int rhsmonth	= 0;
	int rtn			= FAILURE;

	thismonth = getMonthNum(this->TimeStamp);
	if (thismonth > 0) {
		rhsmonth = getMonthNum(rhs.TimeStamp);
		if (rhsmonth > 0) {
			if (thismonth > rhsmonth) {
				rtn = 0;
			} else if(thismonth < rhsmonth) {										
				rtn = 1;
			} else {
				// same month check DD
				if (this->TimeStamp.compare(4, 2, rhs.TimeStamp) < 0) {
					rtn = 1;
				} else if (this->TimeStamp.compare(4, 2, rhs.TimeStamp) == 0) {
					// same Day check HH
					if (this->TimeStamp.compare(7, 2, rhs.TimeStamp) < 0) {
						rtn = 1;
					} else if (this->TimeStamp.compare(7, 2, rhs.TimeStamp) == 0) {
						// same hour check MM
						if (this->TimeStamp.compare(10, 2, rhs.TimeStamp) < 0) {
							rtn = 1;
						} else if (this->TimeStamp.compare(13, 2, rhs.TimeStamp) == 0) {
							// same minute check SS
							if (this->TimeStamp.compare(13, 2, rhs.TimeStamp) < 0) {
								rtn = 1;
							} else {
								rtn = 0;
							}
						} else {
							rtn = 0;
						}
					} else {
						rtn = 0;
					}
				} else {
					rtn = 0;
				}
			}  // end else for same month
		
		} else {
			// fail to get monthnum for rhs.TimeStamp
			rtn = 0;
		}
	
	} else {
		// fail to get monthnum for this->TimeStamp
		rtn = 0;
	}

	return rtn;
}
