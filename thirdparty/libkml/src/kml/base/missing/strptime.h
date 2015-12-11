#include <time.h>

#ifdef _WIN32
char* kml_strptime(const char* buf, const char* fmt, struct tm* tm);
#else
#define kml_strptime strptime
#endif
