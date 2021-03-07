#include "RtcLogWrite.h"

extern void *logObject;

void writelogFunc(const std::string& data)
{

}

std::string getCurrentUTCTime()
{
	time_t rawtime;
	struct tm timeinfo;
	time(&rawtime);
#if defined(_WIN32)
	localtime_s(&timeinfo,&rawtime);
#else
	localtime_r(&rawtime,&timeinfo);
#endif
    char timeFormat[100] = {0};
	strftime(timeFormat, sizeof(timeFormat), "%Y-%m-%d %H:%M:%S", &timeinfo);
	return timeFormat;
}
