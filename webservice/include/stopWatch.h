#ifndef CN_LEANCLOUD_IMAGE_SERVICE_STOP_WATCH_INCLUDE_H_
#define CN_LEANCLOUD_IMAGE_SERVICE_STOP_WATCH_INCLUDE_H_

#include "atomic.h"
#include <stdlib.h>
#include <string>

using namespace std;

typedef enum _watchEvent {
    UNKNOWN = 0,
    READ_FROM_STORE = 1,
    WEB_SERVING = 2,
    CROP_IMAGE = 3,
} WatchEvent;

const size_t MAX_EVENT_COUNT = 4;

class StopWatch {
public:
    StopWatch(WatchEvent event);
    ~StopWatch();
    void setEvent(WatchEvent event);
private:
    int64_t _startTime;
    WatchEvent _event;
};

class MetricsSink {
    class MetricsCounter {
    public:
        MetricsCounter() {
            _event = UNKNOWN;
            _counter = 0;
            _accTime = 0;
        };
        void setEvent(WatchEvent event) {
            _event = event;
        };
        void accTime(const uint64_t value) {
            atomic_inc(&_counter);
            atomic_add(&_accTime, value);
        };
        string stat();
    private:
        WatchEvent _event;
        volatile uint64_t _counter;
        volatile uint64_t _accTime;
    };
public:
    static MetricsSink& getInstance();
    void addRecord(WatchEvent event, const uint64_t timeValue);
    string stat();
private:
    MetricsSink();
    MetricsCounter _counters[MAX_EVENT_COUNT];
};

#endif
