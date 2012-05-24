
#include "CTDSink.h"

void *queryCTD(void* obj){
    CTDSink* sink = (CTDSink*) obj;
    CTDreadings_s* r;

    while(sink->isRunning()){
        r = getCTDValues();
        if(r){
            sink->addNode(r->t,r);
        }
        sleep(1);
    }
    pthread_exit(NULL);
}

CTDSink* CTDSink::m_pInstance = NULL;

CTDSink::CTDSink(){
    pthread_mutex_init(&mapMutex,NULL);
    pthread_mutex_init(&runMutex,NULL);
    running = 1;
    pthread_create(&queryThread,NULL,queryCTD,(void*)this); 
}

CTDSink::~CTDSink(){
    running = 0;
    pthread_join(queryThread,NULL);
    pthread_mutex_destroy(&mapMutex);
    pthread_mutex_destroy(&runMutex);
}

CTDSink* CTDSink::Instance(){
    if(!m_pInstance){
        m_pInstance = new CTDSink();
    }
    return m_pInstance;
}

bool CTDSink::isRunning(){
    bool retVal;
    pthread_mutex_lock(&runMutex);
    retVal = running > 0;
    pthread_mutex_unlock(&runMutex);
    return retVal;
}

void CTDSink::addNode(time_t t, CTDreadings_s *r){
    pthread_mutex_lock(&mapMutex);
    if(nodes.size() >= SINKLIMIT){
        nodes.erase(nodes.begin());
    }
    nodes[t] = *r;
    pthread_mutex_unlock(&mapMutex);
}

CTDreadings_s* CTDSink::getClosestCTDReading(time_t time){
    CTDreadings_s* r = (CTDreadings_s*)malloc(sizeof(CTDreadings_s));

    pthread_mutex_lock(&mapMutex);
    pair<map<time_t,CTDreadings_s>::iterator,map<time_t,CTDreadings_s>::iterator> ranges = nodes.equal_range(time);
    CTDreadings_s reading;
    if(ranges.first != nodes.end() && ranges.second != nodes.end()){
        time_t lowDiff = time - ranges.first->first;
        time_t highDiff = ranges.first->first - time;

        if(lowDiff < highDiff){
            reading = ranges.first->second;
        } else {
            reading = ranges.second->second;
        }
        *r = reading;
    } else {
        if(nodes.size() > 0){
            reading = nodes.rbegin()->second;
            *r = reading;
        } else {
            r = NULL;
        }
    }
    pthread_mutex_unlock(&mapMutex);

    return r;
}

CTDreadings_s* cplusplus_callback_function(time_t time){
    CTDSink* sink = CTDSink::Instance();
    return sink->getClosestCTDReading(time);
}
