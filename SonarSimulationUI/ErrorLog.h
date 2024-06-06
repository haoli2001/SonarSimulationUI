#ifndef ERRORLOG_H
#define ERRORLOG_H

#include <fstream>
#include <iostream>
#include <time.h>
#include <string>
#include <mutex>

using namespace std;

enum LogLevel
{
	debug,
	info,
	warning,
	error
};

class ErrorLog
{
public:
	ErrorLog();
	ErrorLog::ErrorLog(string path, int level);
	~ErrorLog();

	void setLogLevel(LogLevel);
	void closeLog();

	void AddBlockLine();
	void ERROR_log(const string& str);
	void INFO_log(const string& str);
	void WARNING_log(const string& str);
	void DEBUG_log(const string& str);

	string getCurTimeString();

private:
	int logLevel;
	string logPath;
	ofstream m_logOut;

	string _getCurTimeString();
	void output(const string& str, int logLevel);
};


#endif // !ERRORLOG_H

