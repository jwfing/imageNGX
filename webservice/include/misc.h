#ifndef _CN_LEANCLOUD_IMAGESERVICE_COMMON_MISC_INCLUDE_H_
#define _CN_LEANCLOUD_IMAGESERVICE_COMMON_MISC_INCLUDE_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/param.h>
#include <pthread.h>
#include <errno.h>
#include <new>
#include <math.h>

#include "common.h"
#include "atomic.h"


#define len_of_ary(ary) (sizeof(ary)/sizeof((ary)[0]))

inline int is_equal(double a, double b) {
    return fabs((a) - (b)) < EPS ? 1 : 0;
}

inline timeval microseconds_to_tv(const int64_t microseconds) {
    struct timeval tp;

    tp.tv_sec = microseconds / 1000000;
    tp.tv_usec = microseconds % 1000000;

    return tp;
}

inline timespec microseconds_to_ts(const int64_t microseconds) {
    struct timespec ts;

    ts.tv_sec = microseconds / 1000000;
    ts.tv_nsec = (microseconds % 1000000) * 1000;

    return ts;
}

inline int64_t tv_to_microseconds(const timeval & tp) {
    return (((int64_t) tp.tv_sec) * 1000000 + (int64_t) tp.tv_usec);
}

inline int64_t ts_to_microseconds(const timespec & ts) {
    return (((int64_t) ts.tv_sec) * 1000000 + (int64_t) ((ts.tv_nsec + 500) / 1000));
}

inline int64_t get_cur_microseconds_time(void) {
    struct timeval tp;

    gettimeofday(&tp, NULL);

    return tv_to_microseconds(tp);
}

inline void microseconds_sleep(const int64_t microseconds) {
    struct timespec ts = microseconds_to_ts(microseconds);

    nanosleep(&ts, NULL);
}

inline void make_timespec_with_interval(struct timespec& tsp, int64_t useconds) {
    struct timeval now;

    gettimeofday(&now, NULL);
    useconds += now.tv_usec;
    tsp.tv_sec = now.tv_sec;
    while (useconds >= 1000000)
    {
        tsp.tv_sec++;
        useconds -= 1000000;
    }
    tsp.tv_nsec = useconds * 1000;
}

#endif

