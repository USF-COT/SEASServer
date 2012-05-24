
#ifdef __cplusplus
 extern "C" {
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <syslog.h>

// Set Socket Timeout - Sets a timeout in seconds and milliseconds for a given connection.
// NOTE: Set both seconds and milliseconds to 0 for no timeout
void setSocketTimeout(int connection, time_t tv_sec, suseconds_t tv_usec);

#ifdef __cplusplus
 }
#endif
