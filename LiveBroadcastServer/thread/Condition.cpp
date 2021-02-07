//
// Created by rjd67 on 2021/2/7.
//

#include "thread/Condition.h"

Condition::Condition(Mutex& mutex):
	mutex_(mutex)
{
	pthread_cond_init(&cond_, nullptr);
}

Condition::~Condition()
{
	pthread_cond_destroy(&cond_);
}

void Condition::WakeUpOne()
{
	pthread_cond_signal(&cond_);
}

void Condition::WakeUpAll()
{
	pthread_cond_broadcast(&cond_);
}

void Condition::Wait()
{
	pthread_cond_wait(&cond_, mutex_.GetPthreadMutex());
}
