#include "ErrorLog.h"

ErrorLog::ErrorLog()
{
	//string folderPath = ".\\logFile";
	//string command = "mkdir " + folderPath;
	//system(command.c_str());
	//logPath = folderPath + "\\log.log";
	
	logPath = ".\\log.log";
	logLevel = info;

	//app 以追加方式打开
	m_logOut.open(logPath.c_str(), ios::out | ios::app);
}

ErrorLog::ErrorLog(string path,int level)
{
	logPath = path;
	logLevel = level;
	m_logOut.open(logPath.c_str(), ios::out | ios::app);
}


ErrorLog::~ErrorLog()
{
	if (m_logOut.is_open())
	{
		m_logOut.flush();
		m_logOut.close();
	}
}


void ErrorLog::setLogLevel(LogLevel level)
{
	logLevel = level;
}


void ErrorLog::closeLog()
{
	if (m_logOut.is_open())
	{
		m_logOut.flush();
		m_logOut.close();
	}
}

void ErrorLog::ERROR_log(const string& str)
{
	output(str, error);
}

void ErrorLog::WARNING_log(const string& str)
{
	output(str, warning);
}

void ErrorLog::INFO_log(const string& str)
{
	output(str, info);
}

void ErrorLog::DEBUG_log(const string& str)
{
	output(str, debug);
}

void  ErrorLog::AddBlockLine()
{
	string out_str = "\n";
	m_logOut << out_str;
	m_logOut.flush();
}

string ErrorLog::_getCurTimeString()
{
	// 获取当前时间，并规范表示
	char tmp[64];
	time_t ptime;
	time(&ptime);  // time_t time (time_t* timer);
	strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&ptime));
	return tmp;
}

string ErrorLog::getCurTimeString()
{
	// 获取当前时间，并规范表示
	char tmp[64];
	time_t ptime;
	time(&ptime);  // time_t time (time_t* timer);
	strftime(tmp, sizeof(tmp), "_%Y-%m-%d %H-%M-%S", localtime(&ptime));
	return tmp;
}

void ErrorLog::output(const string& str, int level)
{
	static mutex m;
	m.lock();
	if (level >= this->logLevel)
	{
		string level_str;
		if (level == error)
			level_str = " :[ERROR]  :";
		else if (level == info)
			level_str = " :[INFO]   :";
		else if (level == warning)
			level_str = " :[WARNING]:";
		else if (level == debug)
			level_str = " :[DEBUG]  :";

		string out_str = _getCurTimeString() + level_str + str + '\n';
		m_logOut << out_str;
		m_logOut.flush();
	}
	m.unlock();

}