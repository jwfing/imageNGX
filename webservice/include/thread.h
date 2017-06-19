#ifndef CN_LEANCLOUD_IMAGESERVICE_INCLUDE_THREAD_H_
#define CN_LEANCLOUD_IMAGESERVICE_INCLUDE_THREAD_H_

#include <pthread.h>

class Thread
{
public:
    enum
    {
        E_NO_ERROR = 0,
        E_NOT_READY = -1,
        E_FAIL_CREATE = -2,
        E_FAIL_JOIN = -3,
        E_NOT_RUNNING = -4,
        E_DETACH_FAILED = -5,
        E_ALREADY_RUNNING = -6,
        E_INTERNAL = -7,
    };

public:
    Thread();

    virtual ~Thread();

    int start(bool detached = false, void * param = NULL);

    int join();

protected:
    inline bool isRunning() const
    {
        return _isRunning;
    }

    virtual void * _main(void * param) = 0;
    // sample implementation
    //    {
    //        while(isRunning())
    //        {
    //            // do something
    //        }
    //
    //        return 0;
    //    }

private:
    static void * _launcher(void * param);

private:
    typedef enum _ThreadStatus {
        TS_READY = 0,
        TS_RUNNING = 1,
        TS_FINISHED = 2,
    } ThreadStatus;

private:
    bool _detached;
    void *_param;
    volatile bool _isRunning;
    pthread_t _thread;
    pthread_mutex_t _mutex;
    pthread_cond_t  _cond;
    ThreadStatus _status;
    bool _ready;
};

#endif
