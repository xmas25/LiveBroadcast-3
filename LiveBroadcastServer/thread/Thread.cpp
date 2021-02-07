//
// Created by rjd67 on 2021/2/6.
//

#include <cassert>
#include "utils/Logger.h"
#include "thread/Thread.h"

void* ThreadAttr::ThreadFunction(void* arg)
{
	if (arg)
	{
		ThreadAttr* attr = static_cast<ThreadAttr*>(arg);
		if (attr)
		{
			attr->func();
		}
	}
	return nullptr;
}

Thread::Thread(const ThreadFunc& func, const std::string& name):
	started_(false),
	joined_(false),
	attr_({func, name})
{

}

Thread::~Thread()
{
	if (started_ && !joined_)
	{
		pthread_detach(thread_);
	}
}

void Thread::Start()
{
	assert(!started_);
	started_ = true;

	if (pthread_create(&thread_, nullptr, ThreadAttr::ThreadFunction, &attr_) == 0)
	{

	}
	else
	{
		started_ = false;
		LOG_ERROR("start thread failed");
	}
}

int Thread::Join()
{
	assert(started_ && !joined_);
	joined_ = true;
	return pthread_join(thread_, nullptr);
}
