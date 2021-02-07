//
// Created by rjd67 on 2021/2/7.
//

#include <cassert>
#include "Mutex.h"

Mutex::Mutex()
{
	pthread_mutex_init(&mutex_, nullptr);
}

Mutex::~Mutex()
{
	pthread_mutex_destroy(&mutex_);
}

void Mutex::Lock()
{
	pthread_mutex_lock(&mutex_);
}

void Mutex::Unlock()
{
	pthread_mutex_unlock(&mutex_);
}

pthread_mutex_t* Mutex::GetPthreadMutex()
{
	return &mutex_;
}
