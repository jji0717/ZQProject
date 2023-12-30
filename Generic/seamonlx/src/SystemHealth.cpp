/** @file SystemHealth.cpp
 *
 *
 *  SystemHealthObj class constructors and member functions
 *  implementation.
 *  
 *
 *
 *  Revision History
 *  
 *  04-12-2010 mjc   Created ()
 *  05-18-2010 mjc	  Removed InfiniBand() from the list and copy constructor & assignment operator
 *  
 * 
 */

#include <stdexcept>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <string>
#include "common.h"
#include "SystemHealth.h"

using namespace std;
using namespace seamonlx;


/*************************************************
 *
 *       Class SystemHealth methods definition
 * 
 ************************************************/


/**
 * SystemHealth constructor
 */
SystemHealth::SystemHealth()
{
}


/**
 * SystemHealth Destructor
 */ 
SystemHealth::~SystemHealth()
{
}


/**
 * Function update()
 *
 * The function that updates the SystemHealth data members
 */
int
SystemHealth::update()
{
	int retval = SUCCESS;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemHealth::update, Enter");
	

	/*
	 * Check the class instance pointer to each object instantiation and if it's non NULL, then
	 * invoke the objects GetState() method and save of it's state. 
	 */


    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemHealth::update, Exit");
	

	return retval;
}


/* End of SystemHealth.cpp */
