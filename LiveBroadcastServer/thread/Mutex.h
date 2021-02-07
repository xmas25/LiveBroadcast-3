//
// Created by rjd67 on 2021/2/7.
//

#ifndef LIVEBROADCASTSERVER_MUTEX_H
#define LIVEBROADCASTSERVER_MUTEX_H

#include <pthread.h>
class Mutex
{
public:
	Mutex();
	~Mutex();

	void Lock();

	void Unlock();

	pthread_mutex_t* GetPthreadMutex();
private:
	pthread_mutex_t mutex_;
};

class MutexGuardLock
{
public:
	explicit MutexGuardLock(Mutex& mutex):
		mutex_(mutex)
	{
		mutex.Lock();
	}

	~MutexGuardLock()
	{
		mutex_.Unlock();
	}

private:
	Mutex& mutex_;
};

#define MutexGuardLock(x) error "missing guard object name"
#endif //LIVEBROADCASTSERVER_MUTEX_H
