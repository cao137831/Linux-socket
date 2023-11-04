#include <iostream>
#include <string.h>
#include <string>

#include <unistd.h>

#include "ThreadPool.h"

using namespace std;

ThreadPool::ThreadPool(int min, int max)
{
    do 
    {
        // 实例化任务队列
        taskQ = new TaskQueue;
        if (taskQ == nullptr)
        {
            cout <<"malloc taskQ fail...\n";
            return ;
        }
        cout << "实例化任务队列成功\n";

        threadIDs = new pthread_t[max];
        if (threadIDs == nullptr)
        {
            cout <<"malloc threadIDs fail...\n";
            break;
        }
        memset(threadIDs, 0, sizeof(pthread_t) * max);
        minNum = min;
        maxNum = max;
        busyNum = 0;
        liveNum = min;    // 和最小个数相等
        exitNum = 0;

        if (pthread_mutex_init(&mutexPool, NULL) != 0 ||
            pthread_cond_init(&notEmpty, NULL) != 0)
        {
            cout <<"mutex or condition init fail...\n";
            break;
        }

        shutdown = false;

        // 创建线程
        pthread_create(&managerID, NULL, manager, this);
        for (int i = 0; i < min; ++i)
        {
            pthread_create(&threadIDs[i], NULL, worker, this);
        }
        cout <<"当前线程池中的线程数目为: "<< minNum <<'\n';
        cout <<"可与服务器建立连接的客户端的最大数目为: "<< maxNum <<'\n';
        return ;
    } while (0);

    // 释放资源，构造函数执行失败
    if (threadIDs) delete[]threadIDs;
    if (taskQ) delete taskQ;
}

ThreadPool::~ThreadPool()
{
    // 关闭线程池
    shutdown = true;
    // 阻塞回收管理者线程
    pthread_join(managerID, NULL);
    // 唤醒阻塞的消费者线程
    for (int i = 0; i < liveNum; i++)
    {
        pthread_cond_signal(&notEmpty);
    }
    // 释放堆内存
    if (taskQ){
        delete taskQ;
    }
    if (threadIDs){
        delete[]threadIDs;
    }

    pthread_mutex_destroy(&mutexPool);
    pthread_cond_destroy(&notEmpty);
}

void ThreadPool::addTask(Task task)
{
    pthread_mutex_lock(&mutexPool);
    cout <<"任务加入任务队列成功\n";
    if (shutdown)
    {
        pthread_mutex_unlock(&mutexPool);
        return;
    }
    // 添加任务
    taskQ->addTask(task);

    pthread_cond_signal(&notEmpty);
    pthread_mutex_unlock(&mutexPool);
}

int ThreadPool::getBusyNum()
{
    pthread_mutex_lock(&mutexPool);
    int busyNum = busyNum;
    pthread_mutex_unlock(&mutexPool);
    return busyNum;
}

int ThreadPool::getAliveNum()
{
    pthread_mutex_lock(&mutexPool);
    int aliveNum = liveNum;
    pthread_mutex_unlock(&mutexPool);
    return aliveNum;
}

void* ThreadPool::worker(void* arg)
{
    // ThreadPool* pool = (ThreadPool*)arg;
    ThreadPool* pool = static_cast<ThreadPool*>(arg);

    while (true)
    {
        pthread_mutex_lock(&pool->mutexPool);
        // 当前任务队列是否为空
        while (pool->taskQ->taskNumber() == 0 && !pool->shutdown)
        {
            // 阻塞工作线程
            pthread_cond_wait(&pool->notEmpty, &pool->mutexPool);

            // 判断是不是要销毁线程
            if (pool->exitNum > 0)
            {
                pool->exitNum--;
                if (pool->liveNum > pool->minNum)
                {
                    pool->liveNum--;
                    pthread_mutex_unlock(&pool->mutexPool);
                    pool->threadExit();
                }
            }
        }

        // 判断线程池是否被关闭了
        if (pool->shutdown)
        {
            pthread_mutex_unlock(&pool->mutexPool);
            pool->threadExit();
        }

        // 从任务队列中取出一个任务
        Task task = pool->taskQ->takeTask();
        // 解锁
        pool->busyNum++;
        pthread_mutex_unlock(&pool->mutexPool);

        cout <<"thread"<< pthread_self() <<"start working...\n";
        task.function(task.arg);
        cout <<"*(int*)task.arg  :  "<< *(int*)task.arg <<'\n';
        /*
        理应回收这块内存，但是因为产生野指针所以，导致程序到这儿强行停止，所以在此对其添加注释
        delete (int *)task.arg;
        task.arg = nullptr;

        报错:
        free(): invalid pointer
        Aborted (core dumped)

        */

        cout <<"thread"<< pthread_self() <<"end working...\n";
        pthread_mutex_lock(&pool->mutexPool);
        pool->busyNum--;
        pthread_mutex_unlock(&pool->mutexPool);
    }
    return NULL;
}

void* ThreadPool::manager(void* arg)
{
    ThreadPool* pool = static_cast<ThreadPool*>(arg);
    while (!pool->shutdown)
    {
        // 每隔1s检测一次
        sleep(3);

        // 取出线程池中任务的数量和当前线程的数量
        pthread_mutex_lock(&pool->mutexPool);
        int queueSize = pool->taskQ->taskNumber();
        int liveNum = pool->liveNum;
        int busyNum = pool->busyNum;
        cout <<"任务队列中的任务数量: "<< queueSize <<'\n';
        cout <<"当前线程池中存活的线程数量为: "<< liveNum <<'\n';
        cout <<"当前线程池中繁忙的线程数量为: "<< busyNum <<'\n';
        pthread_mutex_unlock(&pool->mutexPool);

        // 添加线程
        // 任务的个数>存活的线程个数 && 存活的线程数<最大线程数
        if (queueSize > 0 && liveNum < pool->maxNum)
        {
            pthread_mutex_lock(&pool->mutexPool);
            int counter = 0;
            for (int i = 0; i < pool->maxNum && counter < NUMBER && pool->liveNum < pool->maxNum; ++i)
            {
                if (pool->threadIDs[i] == 0)
                {
                    pthread_create(&pool->threadIDs[i], NULL, worker, pool);
                    counter++;
                    pool->liveNum++;
                }
            }
            pthread_mutex_unlock(&pool->mutexPool);
        }
        // 销毁线程
        // 忙的线程*2 < 存活的线程数 && 存活的线程>最小线程数
        if (busyNum * 2 < liveNum && liveNum > pool->minNum)
        {
            pthread_mutex_lock(&pool->mutexPool);
            pool->exitNum = NUMBER;
            pthread_mutex_unlock(&pool->mutexPool);
            // 让工作的线程自杀
            for (int i = 0; i < NUMBER; ++i)
            {
                pthread_cond_signal(&pool->notEmpty);
            }
        }
    }
    return NULL;
}

void ThreadPool::threadExit()
{
    pthread_t tid = pthread_self();
    for (int i = 0; i < maxNum; ++i)
    {
        if (threadIDs[i] == tid)
        {
            threadIDs[i] = 0;
            printf("threadExit() called, %ld exiting...\n", tid);
            break;
        }
    }
    pthread_exit(NULL);
}