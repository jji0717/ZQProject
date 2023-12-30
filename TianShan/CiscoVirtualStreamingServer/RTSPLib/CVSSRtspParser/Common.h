/*
 * =====================================================================================
 * 
 *       Filename:  Common.h
 * 
 *    Description:  ���屾��Ŀ����ͷ�ļ��Լ������õ���һЩ���������ݽṹ
 *					����Cisco�ṩ��RTSP spec��׼���
 *        Version:  1.0
 *        Created:  Dec. 8th, 2008
 *       Revision:  
 *       Compiler:  vs.net 2005
 * 
 *         Author:  Xiaoming Li
 *        Company:  
 * 
 * =====================================================================================
 */

#ifndef  COMMON_FILE_HEADER_INC
#define  COMMON_FILE_HEADER_INC

//ZQ Common header
#include "ZQ_common_conf.h"
#include "Locks.h"

//STL header
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <deque>
#include <map>
using namespace std;

typedef list<string>	strlist;
typedef deque<string>	strdeque;
typedef vector<string>	strvector;

#endif   /* ----- #ifndef COMMON_FILE_HEADER_INC  ----- */