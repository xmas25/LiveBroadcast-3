//
// Created by rjd67 on 2021/2/7.
//
#include <cassert>
#include "ThreadPool.h"

ThreadPool::ThreadPool(const std::string& name):
	name_(name),
	started_(false),
	thread_num_(0),
	threads_(),
	mutex_(),
	cond_(mutex_)
{

}

ThreadPool::~ThreadPool()
{
	if (started_)
	{
		Stop();
	}
}

void ThreadPool::SetThreadNum(int num)
{
	assert(!started_);
	thread_num_ = num;
}

void ThreadPool::Start()
{
	assert(!started_);
	started_ = true;

	for (int i = 0; i < thread_num_; ++i)
	{
		threads_.emplace_back( new Thread([this](){
			ThreadFunction();
		}, name_ + "#" + std::to_string(i)));
		threads_[i]->Start();
	}
}

void ThreadPool::Stop()
{
	{
		MutexGuardLock lock(mutex_);
		started_ = false;
		cond_.WakeUpAll();
	}
	for (auto& thread : threads_)
	{
		thread->Join();
	}
}

void ThreadPool::Run(const ThreadPool::TaskFunc& task)
{
	if (task_deque_.empty())
	{
		task();
	}
	else
	{
		{
			MutexGuardLock lock(mutex_);
			task_deque_.push_back(task);
			cond_.WakeUpOne();
		}
	}
}

void ThreadPool::ThreadFunction()
{
	while (started_)
	{
		TaskFunc task(TakeATask());
		if (task)
		{
			task();
		}
	}
}

ThreadPool::TaskFunc ThreadPool::TakeATask()
{
	MutexGuardLock lock(mutex_);
	while (started_ && task_deque_.empty())
	{
		cond_.Wait(); // 线程池启动后且没有任务 则等待
	}

	TaskFunc func;
	if (!task_deque_.empty()) // 当线程池为空且started_=false时 返回空任务
	{
		func = task_deque_.front();
		task_deque_.pop_front();
	}

	return func;
}
