/*
 * @file AlertComponent.h
 *
 *
 *  header file contains AlertComponent Class Def
 *  
 *
 *
 *  Revision History
 
 */

#ifndef ALERCOMPONENT_H
#define ALERCOMPONENT_H

#include <list>

using namespace std;

// ListTemplate Library example using a class.
// The List STL template requires overloading operators =, == and <.
 
class AlertComponent
{
   public:
		string	ComponentName;
		string  TimeStamp;
		string  Severity;
		string  descr;
		string  recc;
	
		AlertComponent();
		AlertComponent(const AlertComponent &);
		~AlertComponent(){};

		AlertComponent &operator=(const AlertComponent &rhs);
		int operator==(const AlertComponent &rhs) const;
		int operator<(const AlertComponent &rhs) const;
};


#endif /* ALERTCOMPONENT_H */
