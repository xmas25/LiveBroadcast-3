//
// Created by rjd67 on 2021/2/7.
//

#ifndef LIVEBROADCASTSERVER_THREADPOOL_H
#define LIVEBROADCASTSERVER_THREADPOOL_H

#include <memory>
#include <vector>
#include <string>
#include <deque>
#include "thread/Thread.h"
#include "thread/Condition.h"

class ThreadPool
{
public:
	typedef std::function<void()> TaskFunc;

	ThreadPool(const std::string& name);
	~ThreadPool();

	void SetThreadNum(int num);

	void Start();

	void Stop();

	void Run(const TaskFunc& task);

private:

	std::string name_;

	bool started_;

	int thread_num_;

	std::vector<std::unique_ptr<Thread>> threads_;

	Mutex mutex_;
	Condition cond_;

	std::deque<TaskFunc> task_deque_;
	void ThreadFunction();

	TaskFunc TakeATask();
};


#endif //LIVEBROADCASTSERVER_THREADPOOL_H
