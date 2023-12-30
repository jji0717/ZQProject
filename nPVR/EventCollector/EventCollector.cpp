// EventCollector.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "AlarmService.h"


DWORD gdwServiceType = 11;

AlarmService		ssApp;

ZQ::common::BaseSchangeServiceApplication * Application = &ssApp;

unsigned long  gdwServiceInstance = 11;
