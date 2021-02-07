//
// Created by rjd67 on 2021/2/7.
//

#ifndef LIVEBROADCASTSERVER_CONDITION_H
#define LIVEBROADCASTSERVER_CONDITION_H

#include "thread/Mutex.h"
class Condition
{
public:
	explicit Condition(Mutex& mutex);
	~Condition();

	void WakeUpOne();

	void WakeUpAll();

	void Wait();
private:

	Mutex& mutex_;

	pthread_cond_t cond_;
};


#endif //LIVEBROADCASTSERVER_CONDITION_H
