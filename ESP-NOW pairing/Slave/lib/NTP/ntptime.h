#include <WiFi.h>


class NtpTime
{
	public:
                void TimeStamp_begin();
                int GetMonth();
                int GetDay();
                int GetYear();
                int GetHour();
                int GetMin();
                int GetSec();
                static bool connectionFlag;

};

