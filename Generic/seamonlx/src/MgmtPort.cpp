/** @file MgmtPort.cpp
 *
 *
 *  MgmtPortObj class constructors and member functions
 *  implementation.
 *  
 *
 *
 *  Revision History
 *  
 *  04-19-2010 mjc   Created
 *  
 * 
 */

#include <stdexcept>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <string>
#include "common.h"
#include "MgmtPort.h"

using namespace std;
using namespace seamonlx;


/*************************************************
 *
 *       Class MgmtPort methods definition
 * 
 ************************************************/


/**
 * MgmtPort constructor
 */
MgmtPort::MgmtPort()
{
}

/**
 * MgmtPort Destructor
 */ 
MgmtPort::~MgmtPort()
{
}


/**
 * Function update()
 *
 * The function that updates the MgmtPort data members
 */
int
MgmtPort::update()
{
	int retval = SUCCESS;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "MgmtPort::update, Enter");
	

	/*
	 * Check the class instance pointer to each object instantiation and if it's non NULL, then
	 * invoke the objects GetState() method and save of it's state. 
	 */


    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "MgmtPort::update, Exit");
	

	return retval;
}


/* End of MgmtPort.cpp */
