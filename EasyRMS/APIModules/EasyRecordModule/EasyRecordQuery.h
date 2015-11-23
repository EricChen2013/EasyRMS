/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       EasyRecordModule.h
    Contains:   EasyDarwin Record Module
*/

#ifndef _EASYRECORDQUERY_H
#define _EASYRECORDQUERY_H

#include <vector>
#include <string>

using namespace std;

class EasyRecordQuery
{
public:
	EasyRecordQuery(const char* path, const char* name);
	virtual ~EasyRecordQuery(void);

public:
	bool List(const char* begin, const char* end, vector<string>* records);

private:
	void GetAllM3U8(vector<string> *records);

	void GetM3U8(const char* begin, const char* end, vector<string>* records);

private:
	static time_t StringToTime(const string time_str); //convert string with formatYYYYMMDDhhmmss to time_t

private:
	string path_;
	string name_;
};

#endif
