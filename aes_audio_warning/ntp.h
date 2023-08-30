#ifndef __NTP_CLIENT_H_
#define __NTP_CLIENT_H_

#include <time.h>
#define TIME_OFFSET			(25200U)	// GMT +7 * 3600

extern void NTP_Init();
extern void NTP_Gettime(tm* time);


#endif	/* __NTP_CLIENT_H_ */