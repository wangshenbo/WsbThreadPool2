// UseDll.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "WsbThreadPool.h"
#include "iostream"
#include "process.h"
#include "assert.h"

shared_ptr<wsb::CWsbThreadPool> mypool = NULL;//线程池

void JobFun(PVOID p)
{
	static int i = 0;
	HANDLE mutex = static_cast<HANDLE>(p);
	WaitForSingleObject(mutex, INFINITE);
	i++;
	std::cout << "My Job:" << i << std::endl;
	ReleaseMutex(mutex);
	
}

unsigned int WINAPI JobSubmitThread(PVOID pParam)
{	
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, L"jobwaitabc");
	WaitForSingleObject(hEvent, INFINITE);
	shared_ptr<wsb::CJob> myjob = wsb::CJob::CreateJob(JobFun, wsb::ThreadPriority::Normal, pParam);
	for (int i = 0; i < 200; i++)
	{
		mypool->SubmitJob(myjob);
	}
	return 0;
}

unsigned int WINAPI JobSubmitThread2(PVOID pParam)
{
	shared_ptr<wsb::CJob> myjob = wsb::CJob::CreateJob(JobFun, wsb::ThreadPriority::Normal, pParam);
	for (int i = 0; i < 20; i++)
	{
		mypool->SubmitJob(myjob);
	}
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (mypool == NULL)
	{
		mypool = wsb::CWsbThreadPool::CreateThreadPool(2, 200);//最小线程数为2，最大线程数为3
	}
	HANDLE mutex = CreateMutex(NULL, false, L"MyJob");
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, L"jobwaitabc");
	for (int i = 0; i < 1300; i++)//第个进程开启的线程数是有限的,再加上线程池的200个线程，总共开启1500个线程
	{
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, JobSubmitThread, mutex, 0, NULL);
		assert(hThread != NULL);
		CloseHandle(hThread);
	}
	SetEvent(hEvent);//开始向线程池提交Job
	for (int i = 0; i < 100000; i++)//开启10万个线程，提交20万次作业，由于线程提交完成以后会退出，所以不存在线程数被限制的问题
	{
		unsigned int ID = 0;
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, JobSubmitThread2, mutex, 0,&ID);
		if (ID==0)//创建线程失败，等待20ms重新创建
		{
			ResetEvent(hEvent);
			WaitForSingleObject(hEvent, 20);
			i--;
		}
		CloseHandle(hThread);
	}
	system("pause");
	return 0;
}

