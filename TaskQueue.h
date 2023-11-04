#pragma once
#include <queue>
#include <pthread.h>

using callback = void(*)(void* arg);

// 任务结构体
struct Task
{
    callback function;
    void* arg;
    Task(){
        function = nullptr;
        arg = nullptr;
    }
    Task(callback f, void* arg){
        this->arg = arg;
        function = f;
    }
};

 
class TaskQueue
{
private:
    std::queue<Task> m_taskQ;
    pthread_mutex_t m_mutex;
public:
    TaskQueue();
    ~TaskQueue();
    // 添加任务
    void addTask(Task task);
    void addTask(callback f, void* arg);
    // 取出任务
    Task takeTask();
    // 获取当前任务个数
    inline size_t taskNumber(){    // 内链函数
        return m_taskQ.size();
    }
};