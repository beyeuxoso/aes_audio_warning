
#ifndef __HELP_STRING_H
#define __HELP_STRING_H
#include <WString.h>
#include <time.h>
extern String getIndex(String data, String strIndex);
extern String getLastIndex(String data, String strIndex);
extern String trimIndex(String data, String strIndex);
extern String trimLastIndex(String data, String strIndex);
extern String DateTimeToString(tm tmstruct);
extern double longTime(String data);

#endif