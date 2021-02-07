//
// Created by rjd67 on 2021/2/6.
//

#ifndef LIVEBROADCASTSERVER_THREAD_H
#define LIVEBROADCASTSERVER_THREAD_H

#include <pthread.h>
#include <functional>
#include <string>

typedef std::function<void()> ThreadFunc;

struct ThreadAttr
{
	ThreadFunc func;

	std::string name;

	static void* ThreadFunction(void* arg);
};

class Thread
{
public:

	/**
	 * 初始化线程
	 * @param func 线程执行函数
	 * @param arg 函数参数
	 */
	Thread(const ThreadFunc& func, const std::string& name);
	~Thread();

	/**
	 * 启动线程 线程只能启动一次
	 */
	void Start();

	int Join();

private:

	pthread_t thread_;

	bool started_;

	bool joined_;

	ThreadAttr attr_;
};


#endif //LIVEBROADCASTSERVER_THREAD_H
