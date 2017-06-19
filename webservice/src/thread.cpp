#include "thread.h"

#include <unistd.h>

Thread::Thread()
{
    _detached = false;
    _param = NULL;
    _isRunning = false;
    _thread = 0;
    _status = TS_READY;
    _ready = false;
    pthread_mutexattr_t mattr;
    if (pthread_mutexattr_init(&mattr) == 0)
    {
#ifdef __x86_64__
        if (pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_NORMAL) == 0)
        {
#endif
            if (0 == pthread_mutex_init(&_mutex, &mattr))
            {
                pthread_cond_init(&_cond, NULL);
                _ready = true;
            }
#ifdef __x86_64__
        }
#endif
        pthread_mutexattr_destroy(&mattr);
    }
}

Thread::~Thread()
{
    if (_ready)
    {
        pthread_cond_destroy(&_cond);
        pthread_mutex_destroy(&_mutex);
        _ready = false;
    }
}

void* Thread::_launcher(void* param)
{
    Thread* thread;
    void* ret = NULL;

    thread = (Thread*)param;

    if (NULL != thread && thread->_ready)
    {
        if (thread->_detached)
        {
            sleep(1);
            if (0 == pthread_detach(pthread_self()))
            {
                // invoke real thread entry
                ret = thread->_main(thread->_param);
            }
            else
            {
                // Unable to detach thread
                ret = (void *) E_DETACH_FAILED;
            }
        }
        else
        {
            // invoke real thread entry
            ret = thread->_main(thread->_param);
        }

        pthread_mutex_lock(&(thread->_mutex));
        thread->_status = TS_FINISHED;
        thread->_isRunning = false;
        pthread_cond_broadcast( &(thread->_cond) );
        pthread_mutex_unlock(&(thread->_mutex));
    }
    else
    {
    }

    return ret;
}

int Thread::start(bool detached, void * param)
{
    int ret = 0;

    if (_ready)
    {
        // Lock
        pthread_mutex_lock(&_mutex);

        if (TS_READY != _status)
        {
            ret = E_ALREADY_RUNNING;
        }
        else
        {
            _detached = detached;
            _param = param;
            _status = TS_RUNNING;
            _isRunning = true;

            if (0 == pthread_create(&_thread, NULL,
                    _launcher, (void *)this))
            {
                ret = E_NO_ERROR;
            }
            else
            {
                _status = TS_READY;
                _isRunning = false;
                ret = E_FAIL_CREATE;
            }
        }

        // Unlock
        pthread_mutex_unlock(&_mutex);
    }
    else
    {
        ret = E_NOT_READY;
    }

    return ret;
}

int Thread::join()
{
    int ret = E_NO_ERROR;

    if (_ready)
    {
        if (TS_READY == _status)
        {
            ret = E_NOT_RUNNING;
        }
        else
        {
            _isRunning = false;

            if (!_detached)
            {
                if (0 == pthread_join(_thread, NULL))
                {
                    ret = E_NO_ERROR;
                }
                else
                {
                    ret = E_FAIL_JOIN;
                }
            }
            else
            {
                ret = E_NO_ERROR;
            }

            // Lock
            pthread_mutex_lock(&_mutex);

            while (TS_RUNNING == _status)
            {
                pthread_cond_wait(&_cond, &_mutex);
            }

            // Unlock
            pthread_mutex_unlock(&_mutex);
            _status = TS_READY;
        }
    }
    else
    {
        ret = E_NOT_READY;
    }

    return ret;
}
