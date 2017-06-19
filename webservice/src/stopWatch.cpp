#include "stopWatch.h"

#include "misc.h"

StopWatch::StopWatch(WatchEvent event) {
    _event = event;
    _startTime = get_cur_microseconds_time();
}

StopWatch::~StopWatch() {
    uint64_t end = get_cur_microseconds_time();
    MetricsSink& sink = MetricsSink::getInstance();
    sink.addRecord(_event, end - _startTime);
}

void StopWatch::setEvent(WatchEvent event) {
    _event = event;
}

MetricsSink& MetricsSink::getInstance() {
    static MetricsSink sink;
    return sink;
}

void MetricsSink::addRecord(WatchEvent event, const uint64_t timeValue) {
    switch (event) {
    case READ_FROM_STORE:
        _counters[1].accTime(timeValue);
        break;
    case WEB_SERVING:
        _counters[2].accTime(timeValue);
        break;
    case CROP_IMAGE:
        _counters[3].accTime(timeValue);
        break;
    default:
        break;
    }
}

string MetricsSink::MetricsCounter::stat() {
    char buffer[128] = {0};
    const char* opName = NULL;
    switch(_event) {
    case READ_FROM_STORE:
        opName = "READ_FROM_MONGO";
        break;
    case WEB_SERVING:
        opName = "WEB_SERVING";
        break;
    case CROP_IMAGE:
        opName = "CROP_IMAGE";
        break;
    default:
        opName = "UNKNOWN";
        break;
    }
    snprintf(buffer, 128, "operationType:%s, count:%lld, accTime(ms):%lld\n", opName, _counter, _accTime/1000);
    return string(buffer);
}

string MetricsSink::stat() {
    string result = "";
    for (size_t index = 1; index < MAX_EVENT_COUNT; ++index) {
        result += _counters[index].stat();
    }
    return result;
}

MetricsSink::MetricsSink() {
    _counters[0].setEvent(UNKNOWN);
    _counters[1].setEvent(READ_FROM_STORE);
    _counters[2].setEvent(WEB_SERVING);
    _counters[3].setEvent(CROP_IMAGE);
}

