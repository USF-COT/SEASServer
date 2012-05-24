/* CTDSink - Runs a thread that, when started, queries the CTD for the latest value.  It will only store up to the limit specified in the constructor.  Once limit is reached, the oldest reading is discarded.  Clients may request the closest value to time x from the sink.
 *
 * By: Michael Lindemuth
 */

#ifndef CTDSINK_HPP
#define CTDSINK_HPP

#include <time.h>
#include <pthread.h>

#include "SEASPeripheralCommands.h"

#define SINKLIMIT 300 


#ifdef __cplusplus

#include <map>

using namespace std;

class CTDSink{

    private:
        pthread_t queryThread;
        pthread_mutex_t mapMutex;
        pthread_mutex_t runMutex;
        map<time_t,CTDreadings_s> nodes;
        unsigned int limit;
        volatile sig_atomic_t running;

        CTDSink();
        CTDSink(CTDSink const&){};
        CTDSink& operator=(CTDSink const*){};
        static CTDSink* m_pInstance;

    public:
        static CTDSink* Instance();
        ~CTDSink();
        bool isRunning();
        void addNode(time_t t, CTDreadings_s *r);
        CTDreadings_s* getClosestCTDReading(time_t time);
};

#else
typedef
struct CTDSink
CTDSink;
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__STDC__) || defined(__cplusplus)
    extern void c_function(CTDSink*);   /* ANSI C prototypes */
    extern CTDreadings_s* cplusplus_callback_function(time_t);
#else
    extern void c_function();        /* K&R style */
    extern CTDreadings_s* cplusplus_callback_function(time_t);
#endif

#ifdef __cplusplus
}
#endif

#endif
